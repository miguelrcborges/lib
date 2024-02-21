.intel_syntax noprefix

.text
.global _start
_start:
	xor rbp, rbp
	pop rdi
	mov rsi, rsp
	mov rdx, rdi
	add rdx, 1
	imul rdx, 8 
	add rdx, rsi
	and rsp, -16
	call main

	mov rdi, rax
	mov rax, 60
	syscall
