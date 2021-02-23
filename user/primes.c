#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define R 0
#define W 1

// 调用pipe方法，会返回两个端口，一个读，一个写，这两个端口放在一个数组里面（2个元素）
int
main(int argc, char *argv[])
{
  int numbers[100], cnt = 0, i;
  int fd[2]; // 用来放两个端口
  for (i = 2; i <= 35; i++) {
    numbers[cnt++] = i;
  }
  while (cnt > 0) {
    pipe(fd); // 创建端口
    int pid = fork();
    if (pid == 0) { // 子进程执行的分支
      int prime, this_prime = 0;
      close(fd[W]); // 将管道的写端口关闭
      cnt = -1;
      while (read(fd[R], &prime, sizeof(prime)) != 0) {
        if (cnt == -1) {
          this_prime = prime;
          cnt = 0;
        } else {
          if (prime % this_prime != 0) numbers[cnt++] = prime;
        }
      }
      printf("prime %d\n", this_prime);
      close(fd[R]); // 关闭管道的读端口
    } else { // 父进程执行的分支
      close(fd[R]); // 关闭管道的读端口
      for (i = 0; i < cnt; i++) {
        write(fd[W], &numbers[i], sizeof(numbers[0]));
      }
      close(fd[W]); // 写完数据以后，关闭写端口
      wait(&pid); // 等待子进程结束
      break;
    }
  }
  exit(0);
}