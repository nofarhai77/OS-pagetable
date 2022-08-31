#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <sys/mman.h>

#include "os.h"

/* ----------------------------------- Assignment details -----------------------------------

this assignment implement simulated OS code that handles a multi-level (trie-based) page table.
our simulated OS targets an imaginary 64-bit x86-like CPU.
pt - pointer to page table (trie) by physical page number.
vpn - virtual page number (withour offset).
ppn - physical page number (withour offset).

# virtual address structure - only the lower 57 bits are used for translation.
							  12 LSB is offset, 7 MSB is sign ext, leave 45 bits to vpn.
# physical address structure - likewise.
# Page table structure - LSB is valid bit, bits 1–11 are unused and must be zero,
						 top 52 bits contain the page frame number that this entry points to.
# trie structure for page table - 5 levels, each with 512 indices of 64-bits.
								  our vpn is 45 bits, divided into 9-bit we get 5 groups of pt nodes.
								  each pt node occupy a physical frame of size 4 KB = 4096 bytes.
							      each a pte = page table entry in node is 64 bits.

------------------------------------------------------------------------------------------*/

/* updating ppn according to given vpn in page table.
if ppn is equal to a special NO MAPPING value, then vpn’s mapping (if it exists) is destroyed. */
void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn)
{

    /* creating pointer to virtual address that suites to physical pt pointer.
    the pointer is to the table root.
    offset 0 is added to customize it to phys_to_virt function. */
    uint64_t *ptPointer = phys_to_virt(pt << 12);

    /* current array index in pt */
    int ptIndex;

    for (int i = 1; i < 5; i++)
    {
        /* getting pte index with 9-bit of ptIndex i using bitwise operation.
        0x1ff in binary is 111111111. */
        ptIndex = (vpn >> (45 - i * 9)) & 0x1ff;

        /* in case valid bit of current pte is 0 */
        if ((ptPointer[ptIndex] & 1) == 0)
        {
            /* vpn’s mapping is already destoried (valid bit is 0) */
            if (ppn == NO_MAPPING)
            {
                return;
            }

            /* construct and update pte according to page table structure.
            allocating page = frame for the next array in page table,
            adding offset zero and sign valid bit to 1. */
            ptPointer[ptIndex] = (alloc_page_frame() << 12) + 1;
        }

        /* update pt pointer to the next array in pt.
        ptPointer[ptIndex] - 1 including frame and offset zero */
        ptPointer = phys_to_virt(ptPointer[ptIndex] - 1);
    }

    /* taking 9-bit of level 4 in vpn (out of 5) */
    ptIndex = (vpn)&0x1ff;

    /* if we got here and ppn == NO_MAPPING so all valid bit in first 4-levels in pt were 1.
    we destroy vpn mapping by put 0 only at level 4 for not destroying others mapping that might be. */
    if (ppn == NO_MAPPING)
    {
        ptPointer[ptIndex] = 0; /* ptPointer is at level 4 */
        return;
    }

    /* if ppn is correct we update last pte to given ppn.
    we add offset zero and sign valid bit to 1.
    ptPointer is at level 4, ptIndex is 9-bit of level 4. */
    ptPointer[ptIndex] = (ppn << 12) + 1;

    return;
}

/* returns the ppn that vpn is mapped to, or NO MAPPING if no mapping exists. */
uint64_t page_table_query(uint64_t pt, uint64_t vpn)
{
    /* creating pointer to virtual address that suites to physical pt pointer.
    the pointer is to the table root.
    offset 0 is added to customize it to phys_to_virt function. */
    uint64_t *ptPointer = phys_to_virt(pt << 12);

    /* current array index in pt */
    int ptIndex;

    for (int i = 1; i < 6; i++)
    {
        /* getting pte index with 9-bit of ptIndex i using bitwise operation.
        0x1ff in binary is 111111111. */
        ptIndex = (vpn >> (45 - i * 9)) & 0x1ff;

        /* in case there is no mapping - valid bit of current pte is 0 */
        if ((ptPointer[ptIndex] & 1) == 0)
        {
            return NO_MAPPING;
        }

        /* unless its the last level - update pt pointer to the next array in pt.
        ptPointer[ptIndex] - 1 including frame and offset zero. */
        if (i != 5)
        {
            ptPointer = phys_to_virt(ptPointer[ptIndex] - 1);
        }
    }

    /* return ppn that suites to given vpn,
    without the 12-bit offset that appear in the physical address. */
    return ptPointer[ptIndex] >> 12;
}