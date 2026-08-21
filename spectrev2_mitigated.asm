global victim_call
global victim_func
global vulnerable_instruction
extern victim_func_addr
extern bounds
extern x
extern target_array

section .text 
victim_func: ; Performs similar load operation of Array like the C-Functions
    lea r8, [rel x]              
    add r8, rdi            ; Index of x[index] 
    movzx r10, byte [r8] 
    movzx r8, byte [r8]            ; Get element
    shl r8, 9               ; multiply by 512 (Element distance in covert channel array)
    add rsi, r8             ; Calculate address of "target_array"
    clflush [rel bounds] 
    mfence
    cmp rdi, [rel bounds]            ; Bounds check 
    jge victim_return       ; Return if index >= bounds 
    lfence                  ; Lfence instruction that mitigates spectre v1
vulnerable_instruction:
    mov rax, [rsi]           ; Spectre v2 mitigated because memory load depends on execution before lfence barrier

victim_return:
    ret 

victim_call:  
    mov rcx, 200                    ; Set number of iterations
    jmp reset_PHR_loop               ; Jump to loop
    align (1<<16)                   ; Align 
    %rep (1<<16)-64                 ; Align label to have last 6 bits equal 0
    nop
    %endrep

reset_PHR_loop:    ; reset PHR to all zeros
    %rep (61)                       ; Fill space 
    nop 
    %endrep 
    dec rcx                         ; Decrease Counter for loop
    jnz reset_PHR_loop              ; Conditional Jump aligned to last 16 bits equaling 0   
    clflush [rel victim_func_addr]
    mfence 
    call [rel victim_func_addr] 
    ret 