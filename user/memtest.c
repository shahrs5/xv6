#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  uint64 total_ram = gettotalmem();
  int mb = (int)(total_ram / (1024 * 1024));

  printf("Total RAM: %d MB\n", mb);
  
  exit(0);
}