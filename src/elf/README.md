ELF 加载器暂不可用。

同样的应用程序代码，以PE格式编译时可以正常工作，但ELF格式下无法产生“可育后代”（fork后子进程无法上台）。

目前尚未探明该问题的原因。疑似 V6++ 分页系统有点问题。


2024.7.20

2051565 GTY
