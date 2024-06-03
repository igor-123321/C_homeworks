    bits 64
    extern malloc, puts, printf, fflush, abort, free
    global main

    section   .data
empty_str: db 0x0
int_format: db "%ld ", 0x0
data: dq 4, 8, 15, 16, 23, 42
data_length: equ ($-data) / 8

    section   .text
;;; print_int proc
; rdi -> heap_list[i]
print_int:
    push rbp
    mov rbp, rsp
    sub rsp, 16

    mov rsi, rdi            ; rsi -> heap_list[i]
    mov rdi, int_format     ; rdi -> int_format
    xor rax, rax            ; rax -> 0
    call printf

    xor rdi, rdi            ; rdi -> 0
    call fflush

    mov rsp, rbp
    pop rbp
    ret

;;; p proc
; rdi -> heap_list[i]
p:
    mov rax, rdi    ; rax -> heap_list[i]
    and rax, 1      ; rax -> heap_list[i] & 0x1
    ret

;;; add_element proc
; rdi -> last element
; rsi -> 0
add_element:
    push rbp
    push rbx
    push r14
    mov rbp, rsp
    sub rsp, 16

    mov r14, rdi        ; r14 -> last element
    mov rbx, rsi        ; rbx -> 0

    mov rdi, 16         ; rdi -> 16
    call malloc         ; rax -> heap_list addr
    test rax, rax       ; true return
    jz abort    

    mov [rax], r14      ; heap_list[0] -> last element
    mov [rax + 8], rbx  ; heap_list[1] -> 0

    mov rsp, rbp
    pop r14
    pop rbx
    pop rbp
    ret

;;; m proc - print list
; rdi -> &heap_list[i]
; rsi -> print_int

m:
    push rbp
    mov rbp, rsp
    sub rsp, 16

    test rdi, rdi       ; heap_list[i] == 0 ? exit : continue
    jz outm

    push rbp
    push rbx

    mov rbx, rdi        ; rbx -> &heap_list[i]
    mov rbp, rsi        ; rbp -> rsi

    mov rdi, [rdi]      ; rdi -> heap_list[i]
    call rsi            ; call print_int

    mov rdi, [rbx + 8]  ; rdi -> &heap_list[i + 1]
    mov rsi, rbp        ; rsi -> print_int
    call m

    pop rbx
    pop rbp

outm:
    mov rsp, rbp
    pop rbp
    ret

;;; f proc - select odd numbers in list1 -> set in new list2
; rdx -> p 
; rsi -> 0
; rdi -> heap_list
; OR
; rdx -> p 
; rsi -> heap_list2
; rdi -> 0
f:
    mov rax, rsi    ; rsi -> 0

    test rdi, rdi   ; heap_list == NULL ? outf : continue
    jz outf

    push rbx        
    push r12
    push r13

    mov rbx, rdi        ; rbx -> heap_list         
    mov r12, rsi        ; r12 -> 0                  
    mov r13, rdx        ; r13 -> p                  

    mov rdi, [rdi]      ; rdi -> heap_list[i]
    call rdx            ; call p, rax -> 0|1
    test rax, rax       ; rax == 0 ? z : continue
    jz z

    mov rdi, [rbx]      ; rdi -> heap_list[i]
    mov rsi, r12        ; rsi -> 0
    call add_element    ; rax -> heap_list2
    mov rsi, rax        ; rsi -> heap_list2
    jmp ff              ; 

z:
    mov rsi, r12        ; rsi -> 0

ff:
    mov rdi, [rbx + 8]  ; rdi -> heap_list[1] = 0
    mov rdx, r13        ; rdx -> p
    call f              ;

    pop r13
    pop r12
    pop rbx

outf:
    ret

;;; main proc
main:
    push rbx

    xor rax, rax                    ; rax -> 0
    mov rbx, data_length            ; rbx -> list length

adding_loop:
    mov rdi, [data + (rbx-1) * 8]   ; rdi -> last element
    mov rsi, rax                    ; rsi -> 0
    call add_element                ; rax -> heap_list

    dec rbx                         ; rbx -> list length - 1
    jnz adding_loop                 ; rbx != 0 ? adding_loop : continue

    mov rbx, rax                    ; rbx -> heap_list

    mov rdi, rax                    ; rdi -> heap_list
    mov rsi, print_int              ; rsi -> print_int
    call m                          ; print heap_list[0], rax -> fflush result

    mov rdi, empty_str              ; rdi -> empty_str
    call puts                       ; print empty string

    mov rdx, p                      ; rdx -> p
    xor rsi, rsi                    ; rsi -> 0
    mov rdi, rbx                    ; rdi -> heap_list
    call f                          ; rax -> heap_list2

    mov rdi, rax                    ; rdi -> heap_list2
    mov rsi, print_int              ; rsi -> print_int
    call m                          ; print heap_list2

    ; start patch memory leak
    call free
    mov rdi, rbx
    call free
    ; end patch memory leak

    mov rdi, empty_str
    call puts

    pop rbx

    xor rax, rax
    ret
