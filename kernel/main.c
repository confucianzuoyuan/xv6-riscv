#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

volatile static int started = 0;

// start() jumps here in supervisor mode on all CPUs.
void
main()
{
  if(cpuid() == 0){ // 判断是否是第0个核
    consoleinit(); // 初始化终端
    printfinit(); // 初始化输出互斥锁
    printf("\n");
    printf("xv6 kernel is booting\n"); // 显示提示信息："xv6 kernel is booting\n"
    printf("\n");
    kinit();         // physical page allocator 初始化物理内存页
    kvminit();       // create kernel page table 创建内核页表
    kvminithart();   // turn on paging 将h/w页表寄存器切换到内核的页表，并启用分页
    procinit();      // process table 初始化进程表
    trapinit();      // trap vectors 设置trap向量
    trapinithart();  // install kernel trap vector 安装内核向量
    plicinit();      // set up interrupt controller 设置中断控制器
    plicinithart();  // ask PLIC for device interrupts 对S模式的hart设置uart启用位，及优先级阈值为0。注：hart指硬件线程。
    binit();         // buffer cache 缓冲区初始化
    iinit();         // inode cache inode缓冲区初始化
    fileinit();      // file table 文件表初始化
    virtio_disk_init(); // emulated hard disk 初始化虚拟磁盘
    userinit();      // first user process 创建第一个用户进程。第一个进程执行一个小程序initcode.S（user/initcode.S:1），该程序通过调用exec系统调用重新进入内核
    __sync_synchronize(); // 同步
    started = 1;
  } else {
    while(started == 0) // 等待核0初始化完成
      ;
    __sync_synchronize(); // 同步
    printf("hart %d starting\n", cpuid()); // 输出核ID信息
    kvminithart();    // turn on paging
    trapinithart();   // install kernel trap vector
    plicinithart();   // ask PLIC for device interrupts
  }

  scheduler();        
}
