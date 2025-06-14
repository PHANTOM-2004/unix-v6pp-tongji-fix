org 0x7c00


; 
; 实模式内存布局
;
; 起始地址    大小       用途
; ----------------------------------------
; 0x000      1KB       中断向量表
; 0x400      256B      BIOS 数据区
; 0x500      29.75KB   可用区域
; 0x7C00     512B      MBR 加载区域
; 0x7E00     607.6KB   可用区域
; 0x9FC00    1KB       扩展 BIOS 数据区
; 0xA0000    64KB      用于彩色显示适配器
; 0xB0000    32KB      用于黑白显示适配器
; 0xB8000    32KB      用于文本显示适配器
; 0XC0000    32KB      显示适配器 BIOS
; 0XC8000    160KB     映射内存
; 0xF0000    64KB-16B  系统BIOS
; 0xFFFF0    16B       系统 BIOS 入口地址
; 


;section .code16
[BITS 16]

start:
  mov ax,cs
  mov ds,ax ; 置数据段地址
  mov es,ax
  mov bp,msg ; es:bp = msg
  mov cx,15 ;串长 = 15
  mov dx,0x1800 ; DH,DL = 起始行、列 = 24,0
  mov bx,0x000c ; BH = 页号 = 0; BL = 样式 = 0xc = 黑底红字
  mov ax,0x1301 ; AH = 0x13 , 调用 0x10 中断功能为显示字符串; AL = 0x1 , 光标跟随移动
  int 0x10 ;调用 BIOS 中断#10 打印文字
loop: 
  jmp loop ;停留至此
msg DB 'Hello ! YT Chen'
times 510-($-$$) DB 0
DW 0xAA55
