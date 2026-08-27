/*
    Author(s): Caleb Ceravolo

    Purpose: 
    To provide overall functions for interfacing with the
    FPGA and debugging. Include functions.h if you want to interface
    with the FPGA
    
    Execution: n/a
*/

#include <stdint.h>
#include <stdio.h>
// #include <stdlib.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include "functions.h"

#define SPIFREQ 10E6
#define SETPERIOD 2
#define GETDATA 1
#define SETUPTIME 0
#define RESETENC 3
// #define TESTING 1
#ifdef TESTING
    int main(int argc, char** argv){
        int output[argc-1];
        intparse(argc-1, argv+1, output);
        //print_bin(8, (*output)<<output[1]);
        // printf("%d", (int)'\0');
        //printf("%X", print);
        for (int i=0; i<argc-1; i++){
            printf("%x ", output[i]);
        }
        printf("\n");
    }
#endif

/*
Mostly a debug function. Used to print out a list
*/
void print_arr(void * input, int len){
    char* ch=(char *)input;
    for (int i=0; i<len; i++){
        printf("%X ", ch[i]);
    }
    printf("\n");
}

/*
Prints the binary representation of a number 
len: Length of output binary number
in: Number to print as binary
*/
void print_bin(int len, int in){
    for (int i=0; i<len; i++){
        printf("%d", ((1<<(len-1-i))&in)&&1);
    }
    printf("\n");
}

/*
Converts a uint32 number to a byte array
*/
void to_char_array(uint32_t base, unsigned char output[4]){
    uint8_t mask=0xFF;
    for (int i=0; i<4; i++){
        output[i]=(base&(mask<<8*(3-i)))>>(8*(3-i));
    }
}

/*
Converts a string to an integer
arg: The pointer to the string
output: int value of string
*/
int string_to_int(char * arg){
    float res = 0;
    uint8_t negative = *arg=='-';
    uint8_t base = 10;
    if (arg[negative]=='0'){
        if (arg[negative+1]=='b'){
            base=2;
        } else if (arg[negative+1]=='x'){
            base=16;
        }
    }
    int curr_num=0;
    for (int i=negative+2*(base!=10); arg[i]!='\0'; i++){
            curr_num=(int)arg[i]-(int)'0';
            if ((base==16)&&(curr_num>9)){
                if (curr_num>9){
                    curr_num-='A'-'9'-1; // Convert from capitol letters to decimal
                    if (curr_num>16){
                        curr_num-='a'-'A'; // Convert from upper case to lower case
                    }
                }
            }
            if (curr_num>=base || curr_num<0){
                printf("ERROR: %s contains character %c which is outside of the accepted range\n", arg, arg[i]);
                exit(1);
            }
            res*=base;
            res+=curr_num;
    }
    return (negative ? -1*res : res);
}

/*
Converts a string to a float
arg: The pointer to the string
output: float value of string
*/
float string_to_float(char * arg){
    float res = 0;
    uint8_t negative = *arg=='-';
    uint8_t decimal = 0;
    uint8_t deci_count = 10;
    int curr_num=0;
    for (int i=negative; arg[i]!='\0'; i++){
        if (arg[i]=='.'){
            decimal=1;
        } else {
            curr_num=(int)(arg[i])-(int)'0';
            if (!decimal){
                res*=10;
                res+=curr_num;
            } else {
                res+=((float)curr_num)/deci_count;
                deci_count*=10;
            }
        }
    }
    return (negative ? -1*res : res);
}

/*
Turns a list of args into floats

Inputs: 

argc: length of args

args: list of strings

output: float list to be overwritten
*/
void floatparse (int argc, char** args, float * output){
    for (int i=0; i<argc; i++){
        output[i]=string_to_float(args[i]);
    }
}
/*
Turns a list of args into ints

Inputs: 

argc: length of args

args: list of strings

output: int list to be overwritten
*/
void intparse (int argc, char** args, int * output){
    for (int i=0; i<argc; i++){
        output[i]=string_to_int(args[i]);
    }
}

/*
Sends raw data out. Not recommended for use
*/
uint32_t fpga_raw(uint32_t outgoing){
    if (wiringPiSPISetup(0, SPIFREQ)<0){
        perror("Setup failed\n");
        return -1;
    }
    uint32_t base = 0;
    unsigned char output[4];
    base=outgoing;
    //print_bin(32, base);
    to_char_array(base, output);
    //print_arr(output, 4);
    wiringPiSPIDataRW(0, output, 4);
    uint32_t result = to_uint_value(output);
    wiringPiSPIClose(0);
    return result;
}

/*
Adjusts the pwm pin at the address to have the given uptime

*/
uint32_t fpga_pwm_uptime(uint8_t motor, uint32_t pwm_uptime){
    if (wiringPiSPISetup(0, SPIFREQ)<0){
        perror("Setup failed\n");
        return -1;
    }
    uint32_t base = 0;
    base|=(SETUPTIME<<26);
    base|=(pwm_uptime);
    base|=(motor<<21);
    return fpga_command(base);
}

/*
Sends a command to the FPGA
*/
uint32_t fpga_command (uint32_t command){
    if (wiringPiSPISetup(0, SPIFREQ)<0){
        perror("Setup failed\n");
        return -1;
    }
    unsigned char output[4];
    uint32_t result1, result2;

    //print_bin(32, base);
    to_char_array(command, output);
    //print_arr(output, 4);
    fpga_fasttran(0, &result1);
    wiringPiSPIDataRW(0, output, 4);
    result2 = to_uint_value(output);
    int errors=0;
    while(result2!=command){
        errors++;
        if (errors>=10){
            printf("Command failed: %i, %i\n", result2, command);
            wiringPiSPIClose(0);
            return result2;
        }
        to_char_array(command, output);
        wiringPiSPIDataRW(0, output, 4);
        result2 = to_uint_value(output);
    }
    wiringPiSPIClose(0);
    return result2;
}
/*
Causes the GPIO pin at the address to have a given period
*/
uint32_t fpga_pwm_period(uint8_t motor, uint32_t pwm_period){
    if (wiringPiSPISetup(0, SPIFREQ)<0){
        perror("Setup failed\n");
        return -1;
    }
    uint32_t base = 0;
    base|=(pwm_period);
    base|=(motor<<21);
    base|=(SETPERIOD<<26);
    return fpga_command(base);
}

uint32_t fpga_reset_encoder(uint8_t encoder){
    uint32_t base = 0;
    base|=(encoder<<21);
    base|=(RESETENC<<26);
    return fpga_command(base);
}
/*
Requests the transfer of data from given data address
*/
uint32_t fpga_datatran(uint8_t data_addr){
    if (wiringPiSPISetup(0, SPIFREQ)<0){
        perror("Setup failed\n");
        return -1;
    }
    uint8_t command = 0b00000001;
    uint32_t base = 0;
    unsigned char output[4];
    base|=data_addr;
    base|=(command<<26);
    //print_bin(32, base);
    to_char_array(base, output);
    // print_arr(output, 4);
    wiringPiSPIDataRW(0, output, 4);
    uint32_t result = to_uint_value(output);
    wiringPiSPIClose(0);
    return result;
}

/*
Safer method to transfer data from pi. Returns the data requested
*/
uint32_t fpga_safetran(uint8_t data_addr){
    if (wiringPiSPISetup(0, SPIFREQ)<0){
        perror("Setup failed\n");
        return -1;
    }
    uint8_t command = 0b00000001;
    uint32_t base = 0;
    unsigned char output[4];
    base|=data_addr;
    base|=(command<<26);

    //print_bin(32, base);
    // print_arr(output, 4);
    to_char_array(base, output);
    wiringPiSPIDataRW(0, output, 4);
    to_char_array(base, output);
    wiringPiSPIDataRW(0, output, 4);
    uint32_t result = to_uint_value(output);
    wiringPiSPIClose(0);
    return result;
}

/*
Fast version of data transfer. Manual opening and closing of spi channel needed
*/
void fpga_fasttran(uint8_t data_addr, uint32_t* result){
    uint8_t command = 0b00000001;
    uint32_t base = 0;
    unsigned char output[4];
    base|=data_addr;
    base|=(command<<26);
    to_char_array(base, output);
    wiringPiSPIDataRW(0, output, 4);
    *result = to_uint_value(output);
}

/*
Converts a character array to a uint32 value
*/
uint32_t to_uint_value(unsigned char input[4]){
    uint32_t out;
    for (int i=0; i<4; i++){
        out|=input[3-i]<<8*i;
    }
    return out;
}

/*
Returns length of a character string
*/
int length_of(char * point){
    int length = 0;
    for (int i=0; point[i]!='\0'; i++){
        length++;
    }
    return length;
}