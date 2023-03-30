#include "os.h"

/* 
    Effective bits = Virtual address size - offset size - sign extension size
    Effective bits = 64 - 12 - 7 = 45
    We can thus use a multi-level PT like the one we saw in class (slide 54 from Lecture 2).
 */
#define LEVEL_COUNT 5
#define LEVEL_BITS 9
#define OFFSET 12
#define CHILDREN 512 // 2 ^ LEVEL_BITS
#define EXIT 1

int handle_no_mapping(uint64_t* directory_base, uint64_t directory_entry, int i);
int handle_mapping(uint64_t* directory_base, uint64_t directory_entry, uint64_t ppn, int i);
uint64_t get_directory_entry(uint64_t vpn, int level);
uint64_t ppn_to_address(uint64_t ppn);
int is_valid(uint64_t ppn);
uint64_t invert_valid_bit(uint64_t address);

void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn) {    
    uint64_t directory_entry;
    uint64_t* directory_base = phys_to_virt(ppn_to_address(pt));
    for (int i = 0; i < LEVEL_COUNT; i++) {
        directory_entry = get_directory_entry(vpn, i);
        if (ppn == NO_MAPPING) {
            if (handle_no_mapping(directory_base, directory_entry, i)) {
                return;
            }
        } else {
            if (handle_mapping(directory_base, directory_entry, ppn, i)) {
                return;
            }
        }
        directory_base = phys_to_virt(invert_valid_bit(directory_base[directory_entry]));
    }
}


uint64_t page_table_query(uint64_t pt, uint64_t vpn) {
    uint64_t directory_entry;
    uint64_t* directory_base = phys_to_virt(ppn_to_address(pt));
    for (int i = 0; i < LEVEL_COUNT; i++) { 
        directory_entry = get_directory_entry(vpn, i);
        if (!is_valid(directory_base[directory_entry])) {
            break;
        }
        if (i == LEVEL_COUNT - 1) {
            return directory_base[directory_entry] >> OFFSET;
        }
        directory_base = phys_to_virt(invert_valid_bit(directory_base[directory_entry]));
    }
    return NO_MAPPING;
}

int handle_no_mapping(uint64_t* directory_base, uint64_t directory_entry, int i) {
    if (i == LEVEL_COUNT - 1) {
        directory_base[directory_entry] = 0;
        return EXIT;
    }
    if (!is_valid(directory_base[directory_entry])) {
        return EXIT;
    }
    return 0;
}

int handle_mapping(uint64_t* directory_base, uint64_t directory_entry, uint64_t ppn, int i) {
    if (i == LEVEL_COUNT - 1) {
        directory_base[directory_entry] = invert_valid_bit(ppn_to_address(ppn));
        return EXIT;
    }
    if (!is_valid(directory_base[directory_entry])) {
        directory_base[directory_entry] = invert_valid_bit(ppn_to_address(alloc_page_frame()));
    }
    return 0;   
}

/*
    Given a VPN and a level index, extracts the relevant directory (PT level) entry key.
    Does so by shifting the unwanted LSBs right, then bitwise ANDing to get rid of unwanted
    MSBs, leaving only the LEVEL_BITS bits we need as the non-zero LSBs.
*/
uint64_t get_directory_entry(uint64_t vpn, int level) {
    int shift_amount = (LEVEL_COUNT - level - 1) * LEVEL_BITS;
    uint64_t shifted_vpn = vpn >> shift_amount;
    uint64_t directory_entry = shifted_vpn & (CHILDREN - 1);
    return directory_entry;
}

uint64_t ppn_to_address(uint64_t ppn) {
    return ppn << OFFSET;
}

int is_valid(uint64_t ppn) {
    return ppn & 1; // 1 iff LSB of ppn is 1
}

uint64_t invert_valid_bit(uint64_t address) {
    return address ^ 1;
}