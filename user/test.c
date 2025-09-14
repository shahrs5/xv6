#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#define SIZE 10

int main(int argc, char *argv[]) {
    printf("Matrix multiplication starting...\n");
    int matrixA[SIZE][SIZE] = {
        {2, 1, 3, 0, 4, 1, 2, 3, 1, 0},
        {1, 3, 2, 1, 0, 2, 4, 1, 3, 2},
        {0, 2, 1, 3, 2, 0, 1, 4, 2, 1},
        {3, 1, 0, 2, 1, 3, 0, 2, 4, 3},
        {2, 0, 4, 1, 3, 2, 1, 0, 1, 2},
        {1, 4, 2, 0, 1, 3, 2, 1, 0, 4},
        {4, 2, 1, 3, 0, 1, 3, 2, 3, 1},
        {0, 1, 3, 2, 4, 0, 1, 3, 2, 0},
        {3, 0, 2, 4, 2, 1, 0, 1, 3, 4},
        {1, 3, 0, 1, 2, 4, 3, 0, 2, 1}
    };

    int matrixB[SIZE][SIZE] = {
        {2, 1, 3, 0, 4, 1, 2, 3, 1, 0},
        {1, 3, 2, 1, 0, 2, 4, 1, 3, 2},
        {0, 2, 1, 3, 2, 0, 1, 4, 2, 1},
        {3, 1, 0, 2, 1, 3, 0, 2, 4, 3},
        {2, 0, 4, 1, 3, 2, 1, 0, 1, 2},
        {1, 4, 2, 0, 1, 3, 2, 1, 0, 4},
        {4, 2, 1, 3, 0, 1, 3, 2, 3, 1},
        {0, 1, 3, 2, 4, 0, 1, 3, 2, 0},
        {3, 0, 2, 4, 2, 1, 0, 1, 3, 4},
        {1, 3, 0, 1, 2, 4, 3, 0, 2, 1}
    };

    int result[SIZE][SIZE];

    // Process one column at a time
    for (int col = 0; col < SIZE; col++) {
        int p[2]; // initalize pipe
        pipe(p);
        
        int pid = fork(); // fork a child process
        
        if (pid == 0) {
            // Child Part
            close(p[0]); 
            
            // Multiplication
            for (int row = 0; row < SIZE; row++) {
                int sum = 0;
                for (int k = 0; k < SIZE; k++) {
                    sum += matrixA[row][k] * matrixB[k][col];
                }
                write(p[1], &sum, sizeof(int));
            }
            
            close(p[1]);
            exit(0);

        } else {
            // Parent
            close(p[1]); // close write end
            
            for (int row = 0; row < SIZE; row++) {
                read(p[0], &result[row][col], sizeof(int));
            }
            
            close(p[0]);
            wait(0); // wait for children to finish
            
            printf("Column %d completed\n", col);
        }
    }
    
    // Print result matrix
    printf("\nResult matrix:\n");
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            printf("%d ", result[i][j]);
        }
        printf("\n");
    }
    
    printf("\nDone!\n");
    exit(0);
}