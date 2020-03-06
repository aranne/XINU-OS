#include <stdio.h>
#include "lab0.h"

extern int etext;   // The first address beyond the end of the text segment.
extern int edata;   // The first address beyond the end of the data segment.
extern int end;     // The first address beyond the end of the BSS segment.

void printsegaddress() {
    kprintf("void printsegaddress()\n\n");
    // Print the addresses of end of the text, data and BSS.
    kprintf("Current: etext[0x%08x]=0x%08x, edata[0x%08x]=0x%08x, ebss[0x%08x]=0x%08x\n"
           ,&etext-1, *(&etext-1), &edata-1, *(&edata-1), &end-1, *(&end-1));

    // Print 4 bytes contents before these addresses.
    kprintf("Preceding: etext[0x%08x]=0x%08x, edata[0x%08x]=0x%08x, ebss[0x%08x]=0x%08x\n"
           ,&etext-2, *(&etext-2), &edata-2, *(&edata-2), &end-2, *(&end-2));

    // Print 4 bytes contents after these addresses.
    kprintf("After: etext[0x%08x]=0x%08x, edata[0x%08x]=0x%08x, ebss[0x%08x]=0x%08x\n\n"
           ,&etext, etext, &edata, edata, &end, end);
}