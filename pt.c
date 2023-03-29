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

uint64_t get_directory_entry(uint64_t vpn, int level);
uint64_t* ppn_to_address(uint64_t ppn);
int is_valid(uint64_t ppn);

void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn) {
    uint64_t directory_entry;
    uint64_t* page_table_base = phys_to_virt(ppn_to_address(pt));
    for (int i = 0; i < LEVEL_COUNT; i++) {
        directory_entry = get_directory_entry(vpn, i);
    }
}


uint64_t page_table_query(uint64_t pt, uint64_t vpn) {
    return 0;
}

/*
    Given a VPN and a level index, extracts the relevant directory (PT level) entry key.
    Does so by shifting the unwanted LSBs right, then bitwise ANDing to get rid of unwanted
    MSBs, leaving only the LEVEL_BITS bits we need.
*/
uint64_t get_directory_entry(uint64_t vpn, int level) {
    int shift_amount = (LEVEL_COUNT - level - 1) * LEVEL_BITS;
    uint64_t shifted_vpn = vpn >> shift_amount;
    uint64_t directory_entry = shifted_vpn & (CHILDREN - 1);
    return directory_entry;
}

uint64_t* ppn_to_address(uint64_t ppn) {
    return ppn << OFFSET;
}

int is_valid(uint64_t ppn) {
    // 1 iff LSB of ppn is 1
    return ppn & 1;
}