.global main
.text
main:
	mov r1, #3
	BL test_bl
	B test_b
fin :
   	mov r1,#0
	swi 0x123456
test_bl:
	mov r1, #2
	mov pc,r14
test_b:
	mov r1,#1
	b fin

