#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// Simple fixed-size matrices
int A[64][64];
int B[64][64];
int C[64][64];

void
multiply(int size)
{
  for(int i = 0; i < size; i++) {
    for(int j = 0; j < size; j++) {
      C[i][j] = 0;
      for(int k = 0; k < size; k++) {
        C[i][j] += A[i][k] * B[k][j];
      }
    }
  }
}

void
init_matrices(int size)
{
  for(int i = 0; i < size; i++) {
    for(int j = 0; j < size; j++) {
      A[i][j] = 1;
      B[i][j] = 2;
    }
  }
}

void
write_to_file(int fd, char *str)
{
  int len = 0;
  while(str[len]) len++;
  write(fd, str, len);
}

void
write_int(int fd, int num)
{
  char buf[20];
  int i = 0;
  
  if(num == 0) {
    write(fd, "0", 1);
    return;
  }
  
  if(num < 0) {
    write(fd, "-", 1);
    num = -num;
  }
  
  // Convert to string in reverse
  while(num > 0) {
    buf[i++] = '0' + (num % 10);
    num /= 10;
  }
  
  // Write in correct order
  while(i > 0) {
    write(fd, &buf[--i], 1);
  }
}

void
run_benchmark(int size)
{
  uint64 start_cycles, end_cycles;
  uint64 start_time, end_time;
  uint64 start_instret, end_instret;
  
  printf("\n=== Matrix Size: %dx%d ===\n", size, size);
  
  // Initialize
  init_matrices(size);
  
  // Measure
  start_cycles = getcycles();
  start_time = gettime();
  start_instret = getinstret();
  
  multiply(size);
  
  end_cycles = getcycles();
  end_time = gettime();
  end_instret = getinstret();
  
  // Print results
  uint64 cycles = end_cycles - start_cycles;
  uint64 time = end_time - start_time;
  uint64 instret = end_instret - start_instret;
  
  printf("Cycles:       %d\n", (int)cycles);
  printf("Time:         %d\n", (int)time);
  printf("Instructions: %d\n", (int)instret);
  
  if(cycles > 0) {
    int ipc = (int)((instret * 100) / cycles);
    printf("IPC:          %d.%d\n", ipc/100, ipc%100);
  }
  
  printf("Result[0][0]: %d (expected %d)\n", C[0][0], size * 2);
}

void
run_benchmark_tofile(int size, int fd)
{
  uint64 start_cycles, end_cycles;
  uint64 start_time, end_time;
  uint64 start_instret, end_instret;
  
  // Initialize
  init_matrices(size);
  
  // Measure
  start_cycles = getcycles();
  start_time = gettime();
  start_instret = getinstret();
  
  multiply(size);
  
  end_cycles = getcycles();
  end_time = gettime();
  end_instret = getinstret();
  
  // Write results to file
  uint64 cycles = end_cycles - start_cycles;
  uint64 time = end_time - start_time;
  uint64 instret = end_instret - start_instret;
  
  write_to_file(fd, "\n=== Matrix Size: ");
  write_int(fd, size);
  write_to_file(fd, "x");
  write_int(fd, size);
  write_to_file(fd, " ===\n");
  
  write_to_file(fd, "Cycles:       ");
  write_int(fd, (int)cycles);
  write_to_file(fd, "\n");
  
  write_to_file(fd, "Time:         ");
  write_int(fd, (int)time);
  write_to_file(fd, "\n");
  
  write_to_file(fd, "Instructions: ");
  write_int(fd, (int)instret);
  write_to_file(fd, "\n");
  
  if(cycles > 0) {
    int ipc = (int)((instret * 100) / cycles);
    write_to_file(fd, "IPC:          ");
    write_int(fd, ipc/100);
    write_to_file(fd, ".");
    write_int(fd, ipc%100);
    write_to_file(fd, "\n");
  }
  
  write_to_file(fd, "Result[0][0]: ");
  write_int(fd, C[0][0]);
  write_to_file(fd, " (expected ");
  write_int(fd, size * 2);
  write_to_file(fd, ")\n");
}

int
main(int argc, char *argv[])
{
  uint64 ram = gettotalmem();
  int fd;
  
  printf("Matrix Multiplication Benchmark\n");
  printf("================================\n");
  printf("Total RAM: %d MB\n\n", (int)(ram / (1024*1024)));
  
  // Test different sizes - print to console
  run_benchmark(8);
  run_benchmark(16);
  run_benchmark(32);
  run_benchmark(64);
  
  printf("\n================================\n");
  printf("Benchmark Complete!\n");
  printf("Saving to question3results.txt...\n");
  
  // Open file for writing
  fd = open("question3results.txt", 0x001 | 0x200 | 0x400);  // O_WRONLY | O_CREATE | O_TRUNC
  if(fd < 0) {
    printf("Error: Cannot create file\n");
    exit(1);
  }
  
  // Write header
  write_to_file(fd, "Matrix Multiplication Benchmark Results\n");
  write_to_file(fd, "========================================\n");
  write_to_file(fd, "Total RAM: ");
  write_int(fd, (int)(ram / (1024*1024)));
  write_to_file(fd, " MB\n");
  
  // Run benchmarks again and save to file
  run_benchmark_tofile(8, fd);
  run_benchmark_tofile(16, fd);
  run_benchmark_tofile(32, fd);
  run_benchmark_tofile(64, fd);
  
  write_to_file(fd, "\n========================================\n");
  write_to_file(fd, "Benchmark Complete!\n");
  
  close(fd);
  
  printf("Results saved to question3results.txt\n");
  
  exit(0);
}