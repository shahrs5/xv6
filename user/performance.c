#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    // Start
    int start_cycles = getcycles();
    int start_time = gettime();
    int start_instret = getinstret();

    // for loop to test out instructions retired and cycles
    int sum = 0;
    for (int i = 0; i < 1000; i++) {
        sum = sum + 1;
    }

    // Finish
    int finish_cycles = getcycles();
    int finish_time = gettime();
    int finish_instret = getinstret();

    // Print results
    printf("Cycles: %d\n", finish_cycles - start_cycles);
    printf("Time: %d\n", finish_time - start_time);
    printf("Instructions Retired: %d\n", finish_instret - start_instret);
}