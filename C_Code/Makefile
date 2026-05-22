all: getD centriAbs fpwm resetEnc toggle all_low colorRead rotateCBy rotateServoTo rotateTo_column raise_lower_column raise_lower_pump tofTest waterLevelCalibrate waterLevelRead 1wTempSensor findVelocity PWM_Map dirtSample

getD: functions.c pins.h FPGAtesting/Get_data.c
	gcc FPGAtesting/Get_data.c functions.c -lwiringPi -o ../Executables/getD
centriAbs: functions.c pins.h FPGAtesting/centriAbs.c
	gcc FPGAtesting/centriAbs.c functions.c -lwiringPi -o ../Executables/centriAbs
fpwm: functions.c pins.h FPGAtesting/Fpwm.c
	gcc FPGAtesting/Fpwm.c functions.c -lwiringPi -o ../Executables/fpwm
resetEnc: functions.c pins.h SystemsTesting/resetEnc.c
	gcc SystemsTesting/resetEnc.c functions.c -lwiringPi -o ../Executables/resetEnc
toggle: functions.c pins.h SystemsTesting/toggle.c
	gcc SystemsTesting/toggle.c functions.c -lwiringPi -o ../Executables/toggle
all_low: functions.c pins.h SystemsTesting/all_low.c
	gcc SystemsTesting/all_low.c functions.c -lwiringPi -o ../Executables/all_low
colorRead: functions.c pins.h SystemsTesting/colorRead.c
	gcc SystemsTesting/colorRead.c functions.c -lwiringPi -o ../Executables/colorRead
rotateCBy: functions.c pins.h SystemsTesting/Centrifuge/rotateCBy.c
	gcc SystemsTesting/Centrifuge/rotateCBy.c functions.c -lwiringPi -o ../Executables/rotateCBy
rotateCTo: functions.c pins.h SystemsTesting/Centrifuge/rotateCTo.c
	gcc SystemsTesting/Centrifuge/rotateCTo.c functions.c -lwiringPi -o ../Executables/rotateCTo
rotateServoTo: functions.c pins.h SystemsTesting/Centrifuge/rotateServoTo.c
	gcc SystemsTesting/Centrifuge/rotateServoTo.c functions.c -lwiringPi -o ../Executables/rotateServoTo
findVelocity: functions.c pins.h SystemsTesting/Centrifuge/findVelocity.c
	gcc SystemsTesting/Centrifuge/findVelocity.c functions.c -lwiringPi -o ../Executables/findVelocity
rotateTo_column: functions.c pins.h SystemsTesting/Column/rotateTo_column.c
	gcc -DBUILD_ROTATETO_MAIN SystemsTesting/Column/rotateTo_column.c functions.c -lwiringPi -o ../Executables/rotateTo_column
raise_lower_column: functions.c pins.h SystemsTesting/Column/raise_lower_column.c
	gcc -DBUILD_RAISELOWER_MAIN SystemsTesting/Column/raise_lower_column.c functions.c -lwiringPi -o ../Executables/raise_lower_column
tofTest: functions.c pins.h SystemsTesting/tof.c SystemsTesting/tofTest.c
	gcc -DBUILD_TOFTEST_MAIN SystemsTesting/tofTest.c SystemsTesting/tof.c functions.c -lwiringPi -o ../Executables/tofTest
raiseLower_pump: functions.c pins.h SystemsTesting/Pump/raiseLower_Pump.c
	gcc SystemsTesting/Pump/raiseLower_Pump.c functions.c -lwiringPi -o ../Executables/raiseLower_pump
tempSensorRead: functions.c pins.h SystemsTesting/tempSensorRead.c
	gcc -DBUILD_TEMPREAD_MAIN SystemsTesting/tempSensorRead.c functions.c pins.h -lwiringPi -o ../Executables/tempSensorRead
1wTempSensor: functions.c pins.h SystemsTesting/1wTempSensor.c
	gcc -DBUILD_1WTEMP_MAIN SystemsTesting/1wTempSensor.c functions.c pins.h -lwiringPi -o ../Executables/1wTempSensor
PWM_Map: functions.c pins.h SystemsTesting/Augur/PWM_Map.c
	gcc -DBUILD_PWMMAP_MAIN SystemsTesting/Augur/PWM_Map.c functions.c pins.h -lwiringPi -o ../Executables/PWM_Map
waterLevelCalibrate: SystemsTesting/WaterLevel/waterLevelCalibrate.c SystemsTesting/WaterLevel/waterLevelADS1015.c
	gcc -DBUILD_WATERLEVELCALIBRATE_MAIN SystemsTesting/WaterLevel/waterLevelCalibrate.c SystemsTesting/WaterLevel/waterLevelADS1015.c -lwiringPi -o ../Executables/waterLevelCalibrate
waterLevelRead: SystemsTesting/WaterLevel/waterLevelRead.c SystemsTesting/WaterLevel/waterLevelADS1015.c
	gcc -DBUILD_WATERLEVELREAD_MAIN SystemsTesting/WaterLevel/waterLevelRead.c SystemsTesting/WaterLevel/waterLevelADS1015.c -lwiringPi -o ../Executables/waterLevelRead
dirtSample: functions.c pins.h SystemsTesting/dirtSample.c 
	gcc -DBUILD_DSAMPLE_MAIN SystemsTesting/dirtSample.c functions.c  -lwiringPi -o ../Executables/dirtSample
