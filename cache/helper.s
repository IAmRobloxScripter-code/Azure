section .text
global write
write:
    push rbp
    mov rbp, rsp

    mov rax, 1
    mov rdi, QWORD [rbp+16]
    mov rsi, QWORD [rbp+24]
    mov rdx, QWORD [rbp+32]
    syscall
    
    pop rbp
    mov rax, 0
    ret