#include "os.h"
#include <stdio.h> //TODO: remove!

/* 
    Effective bits = Virtual address size - offset size - sign extension size
    Effective bits = 64 - 12 - 7 = 45
    We can thus use a multi-level PT like the one we saw in class (slide 54 from Lecture 2).
 */
#define LEVEL_COUNT 5
#define LEVEL_BITS 9
#define OFFSET 12
#define CHILDREN 512 // 2 ^ LEVEL_BITS

uint64_t get_directory_entry(uint64_t vpn, int level);
uint64_t ppn_to_address(uint64_t ppn);
int is_valid(uint64_t ppn);
uint64_t invert_valid_bit(uint64_t address);

void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn) {
    printf("\nPTU: %lx\n", vpn); //FIXME:
    
    uint64_t directory_entry;
    uint64_t* directory_base = phys_to_virt(ppn_to_address(pt));

    for (int i = 0; i < LEVEL_COUNT; i++) {
        directory_entry = get_directory_entry(vpn, i);

        if (ppn == NO_MAPPING) {
            if (i == LEVEL_COUNT - 1) {
                directory_base[directory_entry] = 0;
                return;
            }
            if (!is_valid(directory_base[directory_entry])) {
                return;
            }
            directory_base = phys_to_virt(invert_valid_bit(directory_base[directory_entry]));
        } else {
            if (i == LEVEL_COUNT - 1) {
                directory_base[directory_entry] = invert_valid_bit(ppn_to_address(ppn));
                return;
            }
            if (!is_valid(directory_base[directory_entry])) {
                directory_base[directory_entry] = invert_valid_bit(ppn_to_address(alloc_page_frame()));
            }
        
            directory_base = phys_to_virt(invert_valid_bit(directory_base[directory_entry]));
        }


        // if (is_valid(directory_base[directory_entry]) && i != LEVEL_COUNT - 1) {
        //     directory_base = phys_to_virt(invert_valid_bit(directory_base[directory_entry]));
        //     printf("BASE %lx\n", * directory_base); //FIXME:
        // } else if (ppn != NO_MAPPING) {
        //     uint64_t new_page_number = (i != LEVEL_COUNT - 1) ? alloc_page_frame() : ppn;
        //     directory_base[directory_entry] = invert_valid_bit(ppn_to_address(new_page_number));
        //     printf("UPDATE2 %lx\n", directory_base[directory_entry]); //TODO: remove!
        // } else {
        //     if (is_valid(directory_base[directory_entry])) {
        //         directory_base[directory_entry] = (i == LEVEL_COUNT - 1) ? 0 : directory_base[directory_entry]; // Destroy mapping if on final level
        //         printf("SET %lx\n", directory_base[directory_entry]); //TODO: remove!
        //     }
        //     break; // We break if ppn == NO_MAPPING and either we encounter an invalid mapping or destroy the previous mapping.
        // }
        // if (i != LEVEL_COUNT - 1 && ppn != NO_MAPPING) {
        //     directory_base = phys_to_virt(invert_valid_bit(directory_base[directory_entry]));
        // }

    }
    printf("SET! %lx\n", directory_base[directory_entry]); //FIXME:
}


uint64_t page_table_query(uint64_t pt, uint64_t vpn) {
    printf("\nLOOKUP %lx\n", vpn); //FIXME:
    uint64_t directory_entry;
    uint64_t next_base;
    uint64_t* directory_base = phys_to_virt(ppn_to_address(pt));

    for (int i = 0; i < LEVEL_COUNT; i++) { 
        directory_entry = get_directory_entry(vpn, i);
        // printf("ENTRY %lx\n", directory_entry);
        next_base = directory_base[directory_entry];
        // printf("NEXT %lx\n", next_base);

        if (!is_valid(next_base)) {
            printf("BROKE %lx\n", next_base); //FIXME:
            break;
        }

        if (i == LEVEL_COUNT - 1) {
            printf("RETURNED %lx\n", next_base >> OFFSET); //FIXME:
            return next_base >> OFFSET;
        }
        
        directory_base = phys_to_virt(invert_valid_bit(directory_base[directory_entry]));
    }
    
    return NO_MAPPING;
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