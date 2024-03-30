.global _start
_start:
	xor %rbp, %rbp
	pop %rdi
	mov %rsp, %rsi
	mov %rdi, %rdx
	add $1, %rdx
	imul $8, %rdx
	add %rsi, %rdx
	and $0xfffffffffffffff0, %rsp
	call main

	mov %rax, %rdi
	call die
