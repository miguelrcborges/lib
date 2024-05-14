out_of_memory_str:
	.ascii "Out of memory.\n"
	.set out_of_memory_str_len, .-out_of_memory_str

decommit_err_str:
	.ascii "Failed to decommit memory.\n"
	.set decommit_err_str_len, .-decommit_err_str

release_err_str:
	.ascii "Failed to release memory.\n"
	.set release_err_str_len, .-release_err_str

stack_smash_str:
	.ascii "Stack smashing detected. Exiting.\n"
	.set stack_smash_str_len, .-stack_smash_str

.global die
die:
	mov $60, %eax
	syscall

handle_err:
	mov $1, %eax
	mov $2, %edi
	syscall
	mov $1, %edi
	call die

.global mem_reserve
mem_reserve:
	mov %rdi, %rsi
	mov $9, %eax
	mov $0, %edi
	mov $0, %edx
	mov $34, %r10
	mov $0xffffffffffffffff, %r8
	mov $0, %r9
	syscall
	cmp $0xffffffffffffffff, %rax
	je out_of_memory
	ret

.global mem_rescommit
mem_rescommit:
	mov %rdi, %rsi
	mov $9, %eax
	mov $0, %edi
	mov $3, %edx
	mov $34, %r10
	mov $0xffffffffffffffff, %r8
	mov $0, %r9
	syscall
	cmp $0xffffffffffffffff, %rax
	je out_of_memory 
	ret

.global mem_commit
mem_commit:
	mov $10, %eax
	mov $3, %edx
	syscall
	testq %rax, %rax
	jne out_of_memory
	ret

out_of_memory:
	leaq out_of_memory_str(%rip), %rsi
	mov $out_of_memory_str_len, %edx
	call handle_err

.global mem_decommit
mem_decommit:
	mov $28, %eax
	mov $4, %edx
	syscall
	testq %rax, %rax
	jne .decommit_err 
	ret
.decommit_err:
	leaq decommit_err_str(%rip), %rsi
	mov $decommit_err_str_len, %edx
	call handle_err

.global mem_release
mem_release:
	mov $11, %eax
	syscall
	testq %rax, %rax
	jne .release_err 
	ret
.release_err:
	leaq release_err_str(%rip), %rsi
	mov $release_err_str_len, %edx
	call handle_err


// TODO: may need to change this later
.global mem_getPageSize
mem_getPageSize:
	mov $4096, %eax
	ret

.global __stack_chk_fail
__stack_chk_fail:
	leaq stack_smash_str(%rip), %rsi
	mov $stack_smash_str_len, %edx
	call handle_err
