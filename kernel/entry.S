	# qemu -kernel loads the kernel at 0x80000000
        # and causes each CPU to jump there.
        # kernel.ld causes the following code to
        # be placed at 0x80000000.
.section .text
# _entry开始的指令设置了一个栈，栈空间大小为4KB=(1024*4)，mhartid是运行当前程序的CPU核的ID，其值范围是0～MaxCore-1，因为xv6运行在多核的RISC-V处理器上，系统需要为每个核设置一个堆栈。栈区是从高地址开始向低地址扩展，以16B对齐。
# sp = &stack0  # 将stack0的地址存入sp中
# a0 = 1024*4   # 4KB
# a1 = CPU_ID
# a1 = a1 + 1
# a0 = a0 * a1
# sp = sp + a0  # 栈顶
# 以上代码是19 ～ 24行的伪代码
_entry:
	# set up a stack for C.
        # stack0 is declared in start.c,
        # with a 4096-byte stack per CPU.
        # sp = stack0 + (hartid * 4096)
        la sp, stack0
        li a0, 1024*4
	csrr a1, mhartid
        addi a1, a1, 1
        mul a0, a0, a1
        add sp, sp, a0
	# jump to start() in start.c
        # 调用成功就不再回来了，不成功的话，会进入下面的死循环
        call start
spin:
        j spin
