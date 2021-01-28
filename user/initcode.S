# Initial process that execs /init.
# This code runs in user space.

#include "syscall.h"

# exec(init, argv)
.globl start
start:
        la a0, init # 将init中的地址加载到a0
        la a1, argv # 将argv中的地址加载到a1
        li a7, SYS_exec # exec系统调用对应的数字加载到a7
        ecall # 调用ecall，将控制权交给了操作系统

# userinit会创建初始进程，返回到用户空间，执行刚刚介绍的3条指令，再回到内核空间。这里是任何XV6用户会使用到的第一个系统调用

# for(;;) exit();
exit:
        li a7, SYS_exit # 将exit系统调用对应的数字加载到a7
        ecall # 调用ecall
        jal exit # jal 无条件跳转

# char init[] = "/init\0";
init:
  .string "/init\0"

# char *argv[] = { init, 0 };
.p2align 2
argv:
  .long init
  .long 0
