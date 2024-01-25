.intel_syntax noprefix

// SafePointer mem_reserve(usize size);
// SafePointer mem_rescommit(usize size);
// bool mem_commit(void *ptr, usize size);
// bool mem_decommit(void *ptr, usize size);
// bool mem_release(void *ptr, usize size);

.text
.global mem_reserve
mem_reserve:
	mov rsi, rdi
	mov rax, 9
	mov rdi, 0
	mov rdx, 0
	mov r10, 34
	mov r8, -1
	mov r9, 0
	syscall
	cmp rax, -1
	je .reserve_error
	ret
.reserve_error:
	mov rax, 0
	ret

.global mem_rescommit
mem_rescommit:
	mov rsi, rdi
	mov rax, 9
	mov rdi, 0
	mov rdx, 3 
	mov r10, 34
	mov r8, -1
	mov r9, 0
	syscall
	cmp rax, -1
	je .rescommit_error
	ret
.rescommit_error:
	mov rax, 0
	ret

.global mem_commit
mem_commit:
	mov rax, 10
	mov rdx, 34
	syscall
	ret

.global mem_decommit
mem_decommit:
	mov rax, 28
	mov rdx, 4 
	syscall
	ret

.global mem_release
mem_release:
	mov rax, 11
	syscall
	ret
