all: getD centriAbs fpwm resetEnc toggle all_low colorRead  rotateServoTo rotateTo_column raise_lower_column raiseLower_pump tofTest waterLevelCalibrate 1wTempSensor PWM_Map dirtSample
# Temporarily disabled:
# waterLevelRead

# OLD:
# rotateCBy rotateCTo findVelocity

getD: functions.c pins.h FPGAtesting/Get_data.c
	gcc -DBUILD_GETD_MAIN FPGAtesting/Get_data.c functions.c -lwiringPi -o ../Executables/getD
centriAbs: functions.c pins.h FPGAtesting/centriAbs.c
	gcc -DBUILD_CENTRIABS_MAIN FPGAtesting/centriAbs.c functions.c -lwiringPi -o ../Executables/centriAbs
fpwm: functions.c pins.h FPGAtesting/Fpwm.c
	gcc -DBUILD_FPWM_MAIN FPGAtesting/Fpwm.c functions.c -lwiringPi -o ../Executables/fpwm
resetEnc: functions.c pins.h SystemsTesting/resetEnc.c
	gcc -DBUILD_RESETENC_MAIN SystemsTesting/resetEnc.c functions.c -lwiringPi -o ../Executables/resetEnc
toggle: functions.c pins.h SystemsTesting/toggle.c
	gcc -DBUILD_TOGGLE_MAIN SystemsTesting/toggle.c functions.c -lwiringPi -o ../Executables/toggle
all_low: functions.c pins.h SystemsTesting/all_low.c
	gcc -DBUILD_ALL_LOW_MAIN SystemsTesting/all_low.c functions.c -lwiringPi -o ../Executables/all_low
colorRead: functions.c pins.h SystemsTesting/colorRead.c
	gcc -DBUILD_COLORREAD_MAIN SystemsTesting/colorRead.c functions.c -lwiringPi -o ../Executables/colorRead
# Old centrifuge code
# rotateCBy: functions.c pins.h SystemsTesting/Centrifuge/rotateCBy.c
#	gcc SystemsTesting/Centrifuge/rotateCBy.c functions.c -lwiringPi -o ../Executables/rotateCBy
# rotateCTo: functions.c pins.h SystemsTesting/Centrifuge/rotateCTo.c
#	gcc SystemsTesting/Centrifuge/rotateCTo.c functions.c -lwiringPi -o ../Executables/rotateCTo
rotateServoTo: functions.c pins.h SystemsTesting/Centrifuge/rotateServoTo.c
	gcc -DBUILD_ROTATESERVO_MAIN SystemsTesting/Centrifuge/rotateServoTo.c functions.c -lwiringPi -o ../Executables/rotateServoTo
# Old centrifuge code
# findVelocity: functions.c pins.h SystemsTesting/Centrifuge/findVelocity.c
#	gcc SystemsTesting/Centrifuge/findVelocity.c functions.c -lwiringPi -o ../Executables/findVelocity
rotateTo_column: functions.c pins.h SystemsTesting/Column/rotateTo_column.c
	gcc -DBUILD_ROTATETO_MAIN SystemsTesting/Column/rotateTo_column.c functions.c -lwiringPi -o ../Executables/rotateTo_column
raise_lower_column: functions.c pins.h SystemsTesting/Column/raise_lower_column.c
	gcc -DBUILD_RAISELOWERC_MAIN SystemsTesting/Column/raise_lower_column.c functions.c -lwiringPi -o ../Executables/raise_lower_column
tofTest: functions.c pins.h SystemsTesting/tof.c SystemsTesting/tofTest.c
	gcc -DBUILD_TOFTEST_MAIN SystemsTesting/tofTest.c SystemsTesting/tof.c functions.c -lwiringPi -o ../Executables/tofTest
raiseLower_pump: functions.c pins.h SystemsTesting/Pump/raiseLower_Pump.c
	gcc -DBUILD_RAISELOWERP_MAIN SystemsTesting/Pump/raiseLower_Pump.c functions.c -lwiringPi -o ../Executables/raiseLower_pump
tempSensorRead: functions.c pins.h SystemsTesting/tempSensorRead.c
	gcc -DBUILD_TEMPREAD_MAIN SystemsTesting/tempSensorRead.c functions.c pins.h -lwiringPi -o ../Executables/tempSensorRead
1wTempSensor: functions.c pins.h SystemsTesting/1wTempSensor.c
	gcc -DBUILD_1WTEMP_MAIN SystemsTesting/1wTempSensor.c functions.c pins.h -lwiringPi -o ../Executables/1wTempSensor
PWM_Map: functions.c pins.h SystemsTesting/Augur/PWM_Map.c
	gcc -DBUILD_PWMMAP_MAIN SystemsTesting/Augur/PWM_Map.c functions.c pins.h -lwiringPi -o ../Executables/PWM_Map
waterLevelCalibrate: SystemsTesting/WaterLevel/waterLevelCalibrate.c SystemsTesting/WaterLevel/waterLevelADS1015.c
	gcc -DBUILD_WATERLEVELCALIBRATE_MAIN SystemsTesting/WaterLevel/waterLevelCalibrate.c SystemsTesting/WaterLevel/waterLevelADS1015.c -lwiringPi -o ../Executables/waterLevelCalibrate
# Error: undefined reference to `pct_to_mm' (line 47)
# waterLevelRead: SystemsTesting/WaterLevel/waterLevelRead.c SystemsTesting/WaterLevel/waterLevelADS1015.c
#	gcc -DBUILD_WATERLEVELREAD_MAIN SystemsTesting/WaterLevel/waterLevelRead.c SystemsTesting/WaterLevel/waterLevelADS1015.c -lwiringPi -o ../Executables/waterLevelRead
dirtSample: functions.c pins.h SystemsTesting/Automation/dirtSample.c 
	gcc -DBUILD_DSAMPLE_MAIN SystemsTesting/Automation/dirtSample.c functions.c  -lwiringPi -o ../Executables/dirtSample
centrifugeSpin: functions.c pins.h SystemsTesting/Centrifuge/centrifugeSpin.c
	gcc -DBUILD_CENTRIFUGESPIN_MAIN SystemsTesting/Centrifuge/centrifugeSpin.c functions.c -lwiringPi -o ../Executables/centrifugeSpin
centrifugeFluids: functions.c pins.h SystemsTesting/Automation/centrifugeSpin.c
	gcc -DBUILD_CENTRIFUGEFLUIDS_MAIN SystemsTesting/Automation/centrifugeSpin.c functions.c -lwiringPi -o ../Executables/centrifugeFluids