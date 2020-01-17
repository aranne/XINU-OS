#include <stdio.h>
#include "lab0.h"

extern int etext;   // The first address past the end of the text segment.
extern int edata;   // The first address past the end of the data segment.
extern int end;     // The first address past the end of the BSS segment.

void printsegaddress() {
    // Print the addresses of end of the text, data and BSS.
    kprintf("Address of end of text: 0x%08x\n", &etext-1);
    kprintf("Address of end of data: 0x%08x\n", &edata-1);
    kprintf("Address of end of BSS: 0x%08x\n\n", &end-1);

    // Print 4 bytes contents before these addresses.
    kprintf("Content before the end of text: 0x%08x\n", *(&etext-2));
    kprintf("Content before the end of data: 0x%08x\n", *(&edata-2));
    kprintf("Content before the end of BSS: 0x%08x\n\n", *(&end-2));

    // Print 4 bytes contents after these addresses.
    kprintf("Content after the end of text: 0x%08x\n", etext);
    kprintf("Content after the end of data: 0x%08x\n", edata);
    kprintf("Content after the end of BSS: 0x%08x\n\n", end);
}