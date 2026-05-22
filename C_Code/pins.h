#define LOAD12_1_1 25 // Heating element
#define LOAD12_1_2 24 // DNC
#define LOAD12_1_3 23 // Pump Ninhydrin

#define LOAD12_1_4 22 // Pump KCL
#define LOAD12_1_5 21 // Pump spec
#define LOAD12_1_6 30 // Pump disposal

#define LOAD12_2_1 3 // Fan1
#define LOAD12_2_2 2 // Fan2
#define LOAD12_2_3 0 // Centrifuge


#define CENTRIFUGE_PIN LOAD12_2_3
#define FAN1_PIN LOAD12_1
#define FAN2_PIN LOAD12_2
#define HEAT_ELEMENT_PIN LOAD12_1_1
// #define DNC_PIN LOAD12_1_2
#define NINHYDRIN_PIN LOAD12_1_3
#define KCL_PIN LOAD12_1_4
#define SPEC_PIN LOAD12_1_5
#define DISPOSAL_PIN LOAD12_1_6

// Fluids Pump
#define H1A_1 5  // In
#define H1A_2 4  // Out

#define FLUIDSPUMP_IN_PIN H1A_1
#define FLUIDSPUMP_OUT_PIN H1A_2
// Column Spin
#define H1A_3 1  //
#define H1A_4 16

#define SPIN_COLUMN_PIN1 H1A_3
#define SPIN_COLUMN_PIN2 H1A_4
// Raise Lower Column
#define H42A_1 29  // lower
#define H42A_2 27  // raise

#define COLUMN_RL_PIN1 H42A_1 //lower
#define COLUMN_RL_PIN2 H42A_2 //raise

// #define H18A_2_1 27 // Centrifuge
// #define H18A_2_2 26

// Raise Lower Pump
#define H18A_3_1 11 // Lower
#define H18A_3_2 31 // Raise
#define LOWER_PUMP_PIN H18A_3_1
#define RAISE_PUMP_PIN H18A_3_2


#define BURNER_PIN 0   // placeholder — update to actual wPi pin

#define SDA 8
#define SCL 9
#define MOSI 12
#define MISO 13
#define SCLK 14
#define CE 10
#define ONEWIRE 7

// Encoder channels
#define ENC_CENTRIFUGE_INC 2
#define ENC_CENTRIFUGE_ABS 6
#define ENC_RAISE_LOWER 3
#define ENC_COLUMN_RL_INDEX 1
#define ENC_COLUMN_ROTATE 4

// PWM Channels
#define CENTRIFUGE_SERVO_CHANNEL 0 
#define AUGUR_CHANNEL 1

/* FPGA read channel 7: 32-bit word; bits 0–2 are three Hall inputs (1 = idle, 0 = detected). */
#define HALL_CHANNEL 5
#define COLUMN_TOP_HALL_BIT 0

// Fluids hall effect
#define FLUIDS_HALL_TOP 1
#define FLUIDS_HALL_BOTTOM 2

