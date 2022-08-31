# OS-pagetable
Implementation of simulated OS code that handles a multi-level (trie-based) page table.  
Written in C as an assignment in operating-systems course (Tel Aviv University).

# Properties

Our simulated OS targets an imaginary 64-bit x86-like CPU.  
  
pt - pointer to page table (trie) by physical page number.  
vpn - virtual page number (withour offset).  
ppn - physical page number (withour offset).  

1. Virtual address structure - only the lower 57 bits are used for translation. 12 LSB is offset, 7 MSB is sign ext, leave 45 bits to vpn.
2. Physical address structure - likewise.
3. Page table structure - LSB is valid bit, bits 1–11 are unused and must be zero, top 52 bits contain the page frame number that this entry points to.
4. Trie structure for page table - 5 levels, each with 512 indices of 64-bits. Our vpn is 45 bits, divided into 9-bit we get 5 groups of pt nodes. Each pt node occupy a physical frame of size 4 KB = 4096 bytes. Each a pte = page table entry in node is 64 bits.
