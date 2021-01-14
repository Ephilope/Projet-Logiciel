#test des opérations l'operation MOV,ADD avec les decalages lsl et lsr
.global main
.text
main:
    mov r0, #0x17
    mov r1, #0x30
    add r0, r1, r0, lsl #4
    mov r1, #0x56
    add r3, r1, r0, lsr #4
    mov r1, #0x78
    swi 0x123456
