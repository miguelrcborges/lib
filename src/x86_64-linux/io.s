.intel_syntax noprefix

// bool io_write(usize fd, string s);
// bool io_read(usize fd, u8 *buff, usize len, usize *written);
// bool io_open(string file, u32 mode, usize *fd);
// bool io_close(usize fd);

.text
.global io_write
io_write:
	mov rax, 1 
	syscall
	cmp rax, -1
	xor rax, rax
	sete al
	ret

.global io_close 
io_close:
	mov rax, 3
	syscall
	ret

.global io_read
io_read:
	push rcx
	mov rax, 0
	syscall
	pop rcx
	mov QWORD PTR [rcx], rax
	cmp rax, -1
	xor rax, rax
	sete al
	ret

.global open_syscall
open_syscall:
	mov rax, 2
	syscall
	ret
