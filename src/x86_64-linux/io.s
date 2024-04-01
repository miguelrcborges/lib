.global __open_syscall
__open_syscall:
	mov $2, %rax
	syscall
	ret

.global __write_syscall
__write_syscall:
	mov $1, %rax
	syscall
	ret

.global __read_syscall
__read_syscall:
	mov $0, %rax
	syscall
	ret

.global __close_syscall
__close_syscall:
	mov $3, %rax
	syscall
	ret

.global __fstat_syscall
__fstat_syscall:
	mov $5, %rax
	syscall
	ret
