[BITS 32]
[extern kernelBridge]
; 定义页目录和页表的物理地址
page_directory_base_address equ 0x200000 ; 页目录起始物理地址（2MB）
kernel_page_table_base_address equ 0x201000 ; 内核页表起始物理地址（2MB + 4KB）
global greatstart
greatstart:
; 清空页目录（共 1024 项，每个 4 字节）
mov edi, page_directory_base_address + 0xc0000000
xor eax, eax
mov ecx, 1024
rep stosd
; 设置页目录第 0 和第 768 项指向同一个页表
mov eax, kernel_page_table_base_address
or eax, 0x03 ; Present + Read/Write
mov [page_directory_base_address + 0*4 + 0xc0000000], eax
mov [page_directory_base_address + 768*4 + 0xc0000000], eax
; 初始化内核页表：映射虚拟地址 0xC0000000 ~ 0xC0400000 到物理地址 0x00000000 ~ 0x00400000
mov edi, kernel_page_table_base_address
mov eax, 0x00000003 ; Present + Read/Write
mov ecx, 1024 ; 映射 1024 个页面（4MB）
.fill_kernel_page_table:
mov [edi], eax
add eax, 0x1000 ; 下一个物理页
add edi, 4 ; 下一页表项
loop .fill_kernel_page_table
; 加载空的 IDT（中断描述符表）
lidt [empty_idt]
; 设置 CR3 寄存器，指向页目录基地址
mov eax, page_directory_base_address
mov cr3, eax


; 启用分页机制（CR0.PG = 1）和保护模式（CR0.PE = 1）
mov eax, cr0
or eax, 0x80000001
mov cr0, eax
; 设置段寄存器为内核数据段选择子
mov ax, data_selector
mov ds, ax
mov es, ax
mov ss, ax
; 设置栈指针位于 4MB 处（0xC0400000），确保栈空间已映射
mov esp, 0xc0400000
mov ebp, esp
; 跳转至内核入口函数 kernelBridge
jmp code_selector:kernelBridge
ud2 ; 若跳转失败，触发未定义指令异常用于调试
; 空的 IDT 表
align 4
empty_idt:
.length dw 0
.base dd 0
; GDT 段选择子定义
code_selector equ (1 << 3) ; 内核代码段选择子（GDT 第 1 项）
data_selector equ (2 << 3) ; 内核数据段选择子（GDT 第 2 项）
; GDT 表结构
align 4
gdt_pointer:
dw (gdt_end - gdt_base) - 1 ; GDT 限长
dd gdt_base ; GDT 基地址
dd 0 ; 保留字段
align 4
gdt_base:
dq 0

gdt_kernel_code:
dq 0xcf9a000000ffff ; 内核代码段：DPL=0, 可执行/可读，4GB
gdt_kernel_data:
dq 0xcf92000000ffff ; 内核数据段：DPL=0, 可读/可写，4GB
gdt_end:
