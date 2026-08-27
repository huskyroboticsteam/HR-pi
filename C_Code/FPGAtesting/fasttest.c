/*
    Author: Caleb Ceravolo

    Purpose:
    To stress test the fpga SPI connection using fastran and get error percent
    and execution time of the program

    Execution:
    cd C_Code
    gcc FPGAtesting/fasttest.c functions.c -lwiringPi -o ../Executables/fasttest
    sudo ./Executables/fasttest {num_iterations}
*/

#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdio.h>
#include <stdint.h>
#include "../functions.h"
#include <time.h>

struct timespec start, end;
#define ASCII_ESC 27
#define SPIFREQ 500E3
//#include <signal.h>
int main(int argc, char *argv[]) {

    int vals[argc-1];
    argparse(argc-1, argv+1, vals);
    uint32_t result;
    fpga_datatran(2);
    long int errors = 0;
    long int percent = 0;
    long int lastPercent = -1;
    printf("%c[?25l", ASCII_ESC);
    printf("Testing %i times\n", string_to_int(argv[1]));
    printf("0 Percent");

    if (wiringPiSPISetup(0, SPIFREQ)<0){
        perror("Setup failed\n");
        return -1;
    }
    clock_t start = clock(), diff;
    for (long i=0; i<string_to_int(argv[1]); i++){
        fpga_fasttran(2, &result);
        percent=((i+1)*100)/string_to_int(argv[1]);
        printf("\r%li Percent", percent);
        // if (percent!=lastPercent){
            
        //     lastPercent=percent;
        // }
        
        if (result!=0b11111111111111110000000000000000){
            // printf("%c[%iB\nError: ", ASCII_ESC, errors-1);
            errors++;
            // print_bin(32, result);
            // printf("%c[%iA",ASCII_ESC, errors+1);
        }
    }
    diff = clock() - start;
    wiringPiSPIClose(0);
    FILE* ptr;
      // File opened
    ptr = fopen("./tran_rate.txt", "a");
    fprintf(ptr, "Error rate: %li/%i\n", errors, string_to_int(argv[1]));
    float sec = (float)diff / CLOCKS_PER_SEC;
    fprintf(ptr, "Time elapsed: %fs\n", sec);
    fprintf(ptr, "Average tran rate: %f Mbits/s\n\n", string_to_int(argv[1])*32/(sec*1E6));
    fclose(ptr);
    printf("\nError rate: %li/%i\n", errors, string_to_int(argv[1]));
    printf("%c[?25h", ASCII_ESC);
    printf("Time elapsed: %fs\n", sec);
    printf("Average tran rate: %f Mbits/s\n", string_to_int(argv[1])*32/(sec*1E6));

}
