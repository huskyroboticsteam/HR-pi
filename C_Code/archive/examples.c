
// These are the includes. They tell the script which packages to include
#include <stdio.h>
#include <wiringPi.h>
#include <stdint.h>
#include <stdbool.h>
// This is a define statement, aka a macro. Macros can also be basic functions. 
// It is set on compiling and doesn't change after that
// I use these defines to define which pins my Led and button are connected to
#define LED 0
#define BUTTON 1

// This is a global variable since it was defined outside of a function. Any function can read it or write to it
uint8_t led_status;
// This is the main function. C compiled code always runs the main function
// Argc and argv allow you to accept command line parameters
int main(int argc, char *argv[]){

    // This is a signed int that takes up 32 bits in memory. It cannot be accessed outside of main
    int32_t example_var = 0;

    // This is an unsigned int that takes up 8 bits (1 byte) of memory. 
    // Unsigned means it can't be negative, but the int has twice the maximum size
    uint8_t unsigned_var = 0;

    // This is a boolean. It can either be true or false
    bool value;

    // This sets up the library so we can use the functions
    wiringPiSetup();

    // These tell the program what each pin is for. They can be changed at any time
    pinMode(LED, OUTPUT);
    pinMode(BUTTON, INPUT);
    pullUpDnControl(BUTTON, PUD_UP);

    uint32_t time = 0;
    uint32_t last_toggle = 0;
    // This is a while loop. Since the test is always true, it will run until terminated by linux
    while (1){
        // millis gives the milliseconds from system start. If you need more precision, use micros()
        time = millis();
        if ((time-last_toggle)>1000){
            led_status = digitalRead(LED);

            // The ! command turns a 0 into a non zero and a non zero into a zero
            // For a bitwise inversion, use ~ (if you know what that means)
            digitalWrite(LED, !led_status);

            printf("This is a print statement, useful for printing to the console.\n<This is a newline, causes a new paragraph\n");
            printf("To print variables, use this:%d. Then place what you want to print afterwards\n", example_var);
            last_toggle = time;
        }
    }

    // A return statement from main is used to communicate successes or errors. A 0 means no error.
    return 0;
}