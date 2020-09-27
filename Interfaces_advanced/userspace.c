#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "ioctl_commands.h"

int main() {
  printf("menu\n");
  int fd = open("/proc/plog", O_RDONLY);
  printf("Enter one of following number to execute  corresponding command\n");
  printf("1-Reset Fifo\n");
  printf("2-Reset Stack\n");
  printf("3-Reset All\n");
  int n ;
  char output[30];
  scanf("%d", &n);
  switch (n) {
    case 1:
      ioctl(fd, IOCTL_RESET_FIFO, output);
      printf("fifo is empty now!\n");
      break;
    case 2:
      ioctl(fd, IOCTL_RESET_STACK, output);
      printf("stack is empty now!\n");
      break;
    case 3:
      ioctl(fd, IOCTL_RESET_ALL, output);
      printf("fifo and stack are empty now!\n");
      break;
    default:
      printf("please Enter only one of the above number!\n");
      break;
  }
  close(fd);
  return 0;
}
