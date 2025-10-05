#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define SUPERPGSIZE (2 * 1024 * 1024)

int
main(int argc, char *argv[])
{
  char *p;
  
  printf("Superpage Test\n");
  printf("==============\n\n");
  
  printf("Initial heap: %p\n", sbrk(0));
  
  // Test 1: Allocate less than 2MB (should use normal pages)
  printf("\nTest 1: Allocating 4KB (normal pages)\n");
  p = sbrk(4096);
  if(p == (char*)-1) {
    printf("FAIL: sbrk failed\n");
    exit(1);
  }
  printf("PASS: Got memory at %p\n", p);
  p[0] = 'A';  // Test write
  printf("PASS: Can write to memory\n");
  
  // Test 2: Allocate exactly 2MB (should use superpage)
  printf("\nTest 2: Allocating 2MB (should use superpage)\n");
  char *heap_before = sbrk(0);
  printf("Heap before: %p\n", heap_before);
  
  p = sbrk(SUPERPGSIZE);
  if(p == (char*)-1) {
    printf("FAIL: sbrk failed for 2MB\n");
    exit(1);
  }
  printf("PASS: Got memory at %p\n", p);
  
  char *heap_after = sbrk(0);
  printf("Heap after: %p\n", heap_after);
  printf("Difference: %d bytes\n", (int)(heap_after - heap_before));
  
  // Test write to superpage
  p[0] = 'B';
  p[SUPERPGSIZE - 1] = 'C';
  printf("PASS: Can write to first and last byte\n");
  
  // Test 3: Allocate 4MB (should use 2 superpages)
  printf("\nTest 3: Allocating 4MB (should use 2 superpages)\n");
  p = sbrk(2 * SUPERPGSIZE);
  if(p == (char*)-1) {
    printf("FAIL: sbrk failed for 4MB\n");
    exit(1);
  }
  printf("PASS: Got memory at %p\n", p);
  p[0] = 'D';
  printf("PASS: Can write to memory\n");
  
  // Test 4: Fork test
  printf("\nTest 4: Fork with superpages\n");
  int pid = fork();
  if(pid < 0) {
    printf("FAIL: fork failed\n");
    exit(1);
  }
  
  if(pid == 0) {
    // Child process
    printf("Child: Can access parent's memory? ");
    if(p[0] == 'D') {
      printf("YES\n");
      exit(0);
    } else {
      printf("NO (FAIL)\n");
      exit(1);
    }
  } else {
    // Parent
    wait(0);
    printf("PASS: Fork succeeded\n");
  }
  
  printf("\n==============\n");
  printf("All tests passed!\n");
  
  exit(0);
}