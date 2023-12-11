.text
.global jnihook_gateway
.global jnihook_gateway_size
.extern JNIHook_CallHandler

jnihook_gateway:
        push %rbx
        push %rcx
        push %rdx
        push %rsi
        push %rdi
        push %r8
        push %r9
        push %r10
        push %r11
        push %r12
        push %r13
        push %r14
        push %r15

        mov %rbx, %rdi
        mov %r13, %rsi
        mov %r15, %rdx
        call JNIHook_CallHandler

        pop %r15
        pop %r14
        pop %r13
        pop %r12
        pop %r11
        pop %r10
        pop %r9
        pop %r8
        pop %rdi
        pop %rsi
        pop %rdx
        pop %rcx
        pop %rbx

        jmp *%rax

jnihook_gateway_size: .int . - jnihook_gateway
