[BITS 32]
[extern kernelBridge]

page_directory_base_address equ     0x00200000 ; 页目录的起始物理地址
kernel_page_table_base_address equ  0x00201000 ; 核心态页表的起始物理地址
kernel_vbase equ                    0xc0000000

global greatstart
greatstart:

    ; 页目录清 0 
    mov edi, page_directory_base_address + kernel_vbase ;页目录物理地址（0x200000 + 0xc0000000 + 0x40000000）
    mov ecx, 1024            ; 1024项×4字节 = 4096字节
    xor eax, eax             ;0
    rep stosd 
    
    ; 填写页目录内容
    mov eax, kernel_page_table_base_address
    or eax, 0b000000000011 ; read-write, present
    mov [page_directory_base_address + (768 * 4) + kernel_vbase], eax ; 768#页目录
    mov [page_directory_base_address + kernel_vbase], eax ; 0#页目录
    
    ; 写内核态页表
    mov edi, kernel_page_table_base_address + kernel_vbase ; 内核页表物理地址（0x201000 + 0xc0000000 + 0x40000000）
    mov eax, 0x00000003      ; 物理地址起始页（0x00000000）+权限位（Present | Read/Write）
    mov ecx, 1024            ; 1024 个页表项

.fill_kernel_page_table:
    mov [edi], eax           
    add eax, 0x1000          ; 物理地址递增 4KB
    add edi, 4               ; 页表项指针递增 4 字节
    loop .fill_kernel_page_table
    
    ; 清中断描述符表
    ; load empty idt
    lidt [empty_idt]

    ; 设置 CR3 寄存器
    mov edx, page_directory_base_address
    mov cr3, edx
    
    ; 启动分页和保护机制
    mov ebx, cr0
    or ebx, 0x80000001
    mov cr0, ebx

    mov ax, data_selector
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0xc0400000
    mov ebp, 0xc0400000
    jmp code_selector:kernelBridge
    ud2

align 4 ; 4 字节对齐
empty_idt:
    .length dw 0
    .base dd 0

; 选择子。
code_selector equ (1 << 3)
data_selector equ (2 << 3)

align 4
gdt_pointer:
    dw (gdt_end - gdt_base) - 1 ; limit
    
    dd gdt_base ; base
    dd 0

align 4
gdt_base:
    dq 0
gdt_kernel_code:
    dq 0xcf9a000000ffff
gdt_kernel_data:
    dq 0xcf92000000ffff
gdt_end:
