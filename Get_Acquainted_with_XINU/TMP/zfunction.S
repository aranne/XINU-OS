.text
.globl zfunction

zfunction:
    movl 4(%esp), %eax
    andl $0xFF803FFF, %eax
    shll $0x4, %eax
    ret

