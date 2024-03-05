#include "lib.h"

void __stack_chk_fail(void) {
    io_write(getStdErr(), string("Stack overflow detected. Exiting.\n"));

    __asm__(
        "mov $1, %rdi\n"
        "mov $60, %rax\n"
        "syscall"
    );
}
