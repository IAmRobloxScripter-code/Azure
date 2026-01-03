global main
section .data
    LC0:
        db "gem", 10, 0
section .text
extern write

funnystring:
    push rbp
    mov rbp, rsp
    mov rax, LC0
    pop rbp
    ret
main:
    push rbp
    mov rbp, rsp

    call funnystring
    add rsp, 0
    mov QWORD [rbp-16], rax

    mov rax, 4
    push rax

    mov rax, QWORD [rbp-16]
    push rax

    mov rax, 1
    push rax

    call write
    add rsp, 24
    
    pop rbp
    mov rax, 0
    ret
