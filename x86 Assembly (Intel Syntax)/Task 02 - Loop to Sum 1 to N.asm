; ============================ Your code ============================
section .text
global sum_to_n
sum_to_n:
    ; TODO: sum 1..rdi into rax with a loop
    xor rax, rax
    mov rcx, rdi
    test rcx, rcx
    jz .done

.loop:
    add rax, rcx
    dec rcx
    jnz .loop
    
.done:
    ret

; ===================================================================
; Test harness: reads stdin, calls your code, prints the result.
; You do not need to change anything below this line.
; ===================================================================
global _start
_start:
    call read_input
    call read_int
    mov rdi, rax
    call sum_to_n
    mov rdi, rax
    call print_int
    call print_newline
    jmp exit_ok

; ---------------------------------------------------------------------------
; Runtime: stdin and stdout over raw Linux syscalls. Clobbers only the
; registers the System V ABI lets a callee clobber.
; ---------------------------------------------------------------------------
section .bss
inbuf:   resb 65536
cursor:  resq 1
numbuf:  resb 32
charbuf: resb 1

section .text
; Reads all of stdin into inbuf, NUL-terminated, and points the cursor at it.
read_input:
    push rbx
    xor ebx, ebx
.more:
    mov edx, 65535
    sub rdx, rbx
    jz .done
    xor eax, eax                ; sys_read
    xor edi, edi
    lea rsi, [rel inbuf]
    add rsi, rbx
    syscall
    test rax, rax
    jle .done
    add rbx, rax
    jmp .more
.done:
    lea rsi, [rel inbuf]
    mov byte [rsi + rbx], 0
    mov [rel cursor], rsi
    pop rbx
    ret

; Advances the cursor past spaces, tabs and line breaks; returns it in rsi.
skip_space:
    mov rsi, [rel cursor]
.again:
    movzx eax, byte [rsi]
    cmp al, ' '
    je .next
    cmp al, 9
    je .next
    cmp al, 10
    je .next
    cmp al, 13
    jne .done
.next:
    inc rsi
    jmp .again
.done:
    mov [rel cursor], rsi
    ret

; Reads the next signed decimal integer into rax.
read_int:
    call skip_space
    xor eax, eax
    xor ecx, ecx
    cmp byte [rsi], '-'
    jne .digit
    mov ecx, 1
    inc rsi
.digit:
    movzx edx, byte [rsi]
    sub edx, '0'
    cmp edx, 9
    ja .end
    imul rax, rax, 10
    add rax, rdx
    inc rsi
    jmp .digit
.end:
    mov [rel cursor], rsi
    test ecx, ecx
    jz .ret
    neg rax
.ret:
    ret

; Reads the next whitespace-separated word: rax = its address, rdx = its length.
read_word:
    call skip_space
    mov rax, rsi
.scan:
    cmp byte [rsi], ' '
    jbe .end
    inc rsi
    jmp .scan
.end:
    mov rdx, rsi
    sub rdx, rax
    mov [rel cursor], rsi
    ret

; Returns in rax the rest of the current line, NUL-terminated (its line break
; is overwritten), and moves the cursor to the next line.
read_line:
    mov rax, [rel cursor]
    mov rsi, rax
.scan:
    movzx ecx, byte [rsi]
    test cl, cl
    jz .end
    cmp cl, 10
    je .cut
    inc rsi
    jmp .scan
.cut:
    mov byte [rsi], 0
    inc rsi
.end:
    mov [rel cursor], rsi
    ret

; Writes rdx bytes from rsi to stdout.
print_str:
    mov eax, 1                  ; sys_write
    mov edi, 1
    syscall
    ret

; Writes the byte in dil to stdout.
print_char:
    mov [rel charbuf], dil
    lea rsi, [rel charbuf]
    mov edx, 1
    jmp print_str

print_newline:
    mov edi, 10
    jmp print_char

; Writes rdi as a signed decimal integer.
print_int:
    mov rax, rdi
    lea r8, [rel numbuf + 32]
    mov rsi, r8
    xor r9d, r9d
    test rax, rax
    jns .convert
    neg rax
    mov r9d, 1
.convert:
    mov ecx, 10
.digit:
    xor edx, edx
    div rcx
    add dl, '0'
    dec rsi
    mov [rsi], dl
    test rax, rax
    jnz .digit
    test r9d, r9d
    jz .out
    dec rsi
    mov byte [rsi], '-'
.out:
    mov rdx, r8
    sub rdx, rsi
    jmp print_str

exit_ok:
    mov eax, 60                 ; sys_exit
    xor edi, edi
    syscall