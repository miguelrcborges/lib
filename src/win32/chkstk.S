// https://github.com/skeeto/w64devkit/blob/master/src/libchkstk.S
#if __amd64
#  if CHKSTK_MS
	.globl ___chkstk_ms
___chkstk_ms:
#  elif CHKSTK
	.globl __chkstk
__chkstk:
#  endif
	push %rax
	push %rcx
	mov  %gs:(0x10), %rcx	// rcx = stack low address
	neg  %rax		// rax = frame low address
	add  %rsp, %rax		// "
	jb   1f			// frame low address overflow?
	xor  %eax, %eax		// overflowed: frame low address = null
0:	sub  $0x1000, %rcx	// extend stack into guard page
	test %eax, (%rcx)	// commit page (two instruction bytes)
1:	cmp  %rax, %rcx
	ja   0b
	pop  %rcx
	pop  %rax
	ret
#endif  // __amd64

#if __i386
#  if CHKSTK_MS
// Behaves exactly like x64 ___chkstk_ms.
	.globl ___chkstk_ms
___chkstk_ms:
	push %eax
	push %ecx
	mov  %fs:(0x08), %ecx	// ecx = stack low address
	neg  %eax		// eax = frame low address
	add  %esp, %eax		// "
	jb   1f			// frame low address overflow?
	xor  %eax, %eax		// overflowed: frame low address = null
0:	sub  $0x1000, %ecx	// extend stack into guard page
	test %eax, (%ecx)	// commit page (two instruction bytes)
1:	cmp  %eax, %ecx
	ja   0b
	pop  %ecx
	pop  %eax
	ret
#  elif CHKSTK
// On x86, __chkstk allocates the new stack frame. This implementation
// clobbers eax. MSVC only seems to care about ebp and ecx (this).
	.globl __chkstk
__chkstk:
	push %ecx		// preserve ecx
	mov  %fs:(0x08), %ecx	// ecx = stack low address
	neg  %eax		// eax = frame low address
	lea  8(%esp,%eax), %eax	// "
	cmp  %esp, %eax		// frame low address overflow?
	jb   1f			// "
	xor  %eax, %eax		// overflowed: frame low address = null
0:	sub  $0x1000, %ecx	// extend stack into guard page
	test %eax, (%ecx)	// commit page (two instruction bytes)
1:	cmp  %eax, %ecx
	ja   0b
	pop  %ecx		// restore ecx
	xchg %eax, %esp		// allocate frame
	jmp  *(%eax)		// return
#  endif
#endif  
