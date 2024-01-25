.intel_syntax noprefix

// SafePointer mem_reserve(usize size);
// bool mem_commit(void *ptr, usize size);
// bool mem_decommit(void *ptr, usize size);
// bool mem_release(void *ptr, usize size);
mem_reserve:
	mov rsi, rdi
	mov rax, 9
	mov rdi, 0
	mov
