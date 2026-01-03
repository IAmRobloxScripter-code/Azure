global _start
default rel
section .text
extern main
_start:
    call main
    mov rdi, rax
    mov rax, 60
    syscall