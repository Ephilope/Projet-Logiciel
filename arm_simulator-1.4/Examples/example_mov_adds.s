#test des opéartions MOV, ADDS, SUBS et ANDEQ
.global main
.text
main:
    mov r0, #0x12
    adds r1 , r1 , r2
    mov r2, #0x512
    mov r3, #0x500
    subs r2 , r2 , r3
    andeq r2 , r2 , r0 , lsl #7
    swi 0x123456
    
