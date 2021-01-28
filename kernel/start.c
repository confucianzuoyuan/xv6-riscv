#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

void main();
void timerinit();

// entry.S needs one stack per CPU.
// 每一个cpu都要有一个栈空间
__attribute__ ((aligned (16))) char stack0[4096 * NCPU];

// a scratch area per CPU for machine-mode timer interrupts.
uint64 timer_scratch[NCPU][5];

// assembly code in kernelvec.S for machine-mode timer interrupt.
extern void timervec();

// entry.S jumps here in machine mode on stack0.
// RISC-V的架构有机器模式（Machine Mode，M模式）、监督模式(Supervisor Mode，S模式）和用户模式（User Mode，U模式），并且规定机器模式是必须具备的模式，其他模式均是可选而非必选的模式。entry.S和start()函数都是在机器模式下执行。start对环境进行必要的配置后通过mret指令切换到监督模式，并执行main函数。
void
start()
{
  // set M Previous Privilege mode to Supervisor, for mret.
  unsigned long x = r_mstatus(); // 读mstatus寄存器的值并保存在x中
  x &= ~MSTATUS_MPP_MASK;
  x |= MSTATUS_MPP_S;
  w_mstatus(x);

  // set M Exception Program Counter to main, for mret.
  // requires gcc -mcmodel=medany
  // 将main的地址看作机器模式下发生异常时指令的地址并保存在寄存器mepc，当执行mret指令时，程序从发生异常的指令处恢复执行
  w_mepc((uint64)main);

  // disable paging for now.
  // 将0写入页表寄存器satp来禁用虚拟地址转换
  w_satp(0);

  // delegate all interrupts and exceptions to supervisor mode.
  // 将所有中断和异常委托给监督模式
  w_medeleg(0xffff);
  w_mideleg(0xffff);
  w_sie(r_sie() | SIE_SEIE | SIE_STIE | SIE_SSIE);

  // ask for clock interrupts.
  // 对时钟芯片编程以产生计时器中断
  timerinit();

  // keep each CPU's hartid in its tp register, for cpuid().
  int id = r_mhartid(); // 读取核的ID
  w_tp(id); // 将ID保存到tp寄存器中

  // switch to supervisor mode and jump to main().
  // 执行mret指令，系统由机器模式改变为监督模式，并从main()开始运行
  asm volatile("mret");
}

// set up to receive timer interrupts in machine mode,
// which arrive at timervec in kernelvec.S,
// which turns them into software interrupts for
// devintr() in trap.c.
void
timerinit()
{
  // each CPU has a separate source of timer interrupts.
  int id = r_mhartid();

  // ask the CLINT for a timer interrupt.
  int interval = 1000000; // cycles; about 1/10th second in qemu.
  *(uint64*)CLINT_MTIMECMP(id) = *(uint64*)CLINT_MTIME + interval;

  // prepare information in scratch[] for timervec.
  // scratch[0..2] : space for timervec to save registers.
  // scratch[3] : address of CLINT MTIMECMP register.
  // scratch[4] : desired interval (in cycles) between timer interrupts.
  uint64 *scratch = &timer_scratch[id][0];
  scratch[3] = CLINT_MTIMECMP(id);
  scratch[4] = interval;
  w_mscratch((uint64)scratch);

  // set the machine-mode trap handler.
  w_mtvec((uint64)timervec);

  // enable machine-mode interrupts.
  w_mstatus(r_mstatus() | MSTATUS_MIE);

  // enable machine-mode timer interrupts.
  w_mie(r_mie() | MIE_MTIE);
}
