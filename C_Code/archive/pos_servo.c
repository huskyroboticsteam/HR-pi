#include <wiringPi.h>
#include <stdio.h>
#include <stdint.h>

#define pin 23
int length_of(char *);

/*
Takes the servo angle as an argument
*/
int main(int argc, char *argv[]) {
    int angle = 0;
    int val;
    int argument;
    int length = length_of(argv[1]);
    int dec = 1;
    for (int i=length-1; i>=0; i--){
        argument = (int)(*(argv[1]+i))-(int)'0';
        angle+=dec*argument;
        dec*=10;
    }
    angle%=181;
    val = 50 + angle*(10/9);
    //uint8_t val = argument;
    // if (argument==0){
    //     val = 100;
    // } else if (argument==1){
    //     val= 200;
    // } else {
    //     val = 150;
    // }
    wiringPiSetupPinType(WPI_PIN_WPI);  // must be hardware PWM pin
    if (getAlt(pin)!=4 || getAlt(pin)!=2){
        pinMode(pin, PWM_OUTPUT);
        pwmSetMode(PWM_MODE_MS);     // mark-space mode
        pwmSetClock(192);            // 19.2 MHz / 192 = 100 kHz
        pwmSetRange(2000);           // 100 kHz / 2000 = 50 Hz
    }
    pwmWrite(pin, 2000-val);          // 150/2000 = 7.5% = 1.5 ms pulse (neutral servo)
}

int length_of(char * point){
    int length = 0;
    for (int i=0; point[i]!='\0'; i++){
        length++;
    }
    return length;
}
int to_int(char * input){
    int val = 0;
    int argument;
    int length = length_of(input);
    int dec = 1;
    for (int i=length-1; i>=0; i--){
        argument = (int)(input[i])-(int)'0';
        val+=dec*argument;
        dec*=10;
    }
    return val;
}