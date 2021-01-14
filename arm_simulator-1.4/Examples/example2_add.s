.global main
.text
main:
    mov r0, #0x11
    mov r1, #0x30
    mov r2, #1
    add r0, r0, r1, lsr r2
    mov r0, #25
    sub r0, r0, r1, lsl r2
    swi 0x123456
