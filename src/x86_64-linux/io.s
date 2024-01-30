.intel_syntax noprefix

// usize io_open(s8 file, u32 flags, u32 mode);
// bool io_write(usize fd, const char *buf, usize len);
// bool io_read(usize fd, char *buff, usize len, usize *written);

.text
.global io_write
io_write:
	mov rax, 1 
	syscall
	cmp rax, -1
	xor rax, rax
	sete rax
	ret

.global io_read 
io_read:
	push rcx
	syscall
	pop rcx
	cmp rax, -1
	je .read_error
	mov QWORD PTR [rcx], rax
	mov rax, 0
	ret
.read_error:
	mov QWORD PTR [rcx], 0
	mov rax, 1
	ret
