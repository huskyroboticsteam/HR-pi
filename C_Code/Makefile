all: getD getRaw centriAbs fpwm softPWM resetEnc toggle all_low colorReadTest rotateServoTo rotateTo_column raise_lower_column raise_lower_column_acc tofTest raiseLower_pump tempSensorRead 1wTempSensor PWM_Map pumpToHeight waterLevelRead dirtSample burnerController colorRead 
spectrometer: calibrate_spectrometer stream_spec_final
# OLD:
# rotateCBy rotateCTo findVelocity

getD: functions.c pins.h FPGAtesting/Get_data.c
	gcc -DBUILD_GETD_MAIN FPGAtesting/Get_data.c functions.c -lwiringPi -o ../Executables/getD
getRaw: functions.c pins.h FPGAtesting/GetRaw.c
	gcc -DBUILD_GETD_MAIN FPGAtesting/GetRaw.c functions.c -lwiringPi -o ../Executables/getRaw
centriAbs: functions.c pins.h FPGAtesting/centriAbs.c
	gcc -DBUILD_CENTRIABS_MAIN FPGAtesting/centriAbs.c functions.c -lwiringPi -o ../Executables/centriAbs
fpwm: functions.c pins.h FPGAtesting/Fpwm.c
	gcc -DBUILD_FPWM_MAIN FPGAtesting/Fpwm.c functions.c -lwiringPi -o ../Executables/fpwm
softPWM: functions.c pins.h SystemsTesting/softPWM.c
	gcc SystemsTesting/softPWM.c functions.c -lwiringPi -o ../Executables/softPWM
resetEnc: functions.c pins.h SystemsTesting/resetEnc.c
	gcc -DBUILD_RESETENC_MAIN SystemsTesting/resetEnc.c functions.c -lwiringPi -o ../Executables/resetEnc
toggle: functions.c pins.h SystemsTesting/toggle.c
	gcc -DBUILD_TOGGLE_MAIN SystemsTesting/toggle.c functions.c -lwiringPi -o ../Executables/toggle
all_low: functions.c pins.h SystemsTesting/all_low.c
	gcc -DBUILD_ALL_LOW_MAIN SystemsTesting/all_low.c functions.c -lwiringPi -o ../Executables/all_low
colorReadTest: functions.c pins.h SystemsTesting/colorReadTest.c
	gcc -DBUILD_COLORREADTEST_MAIN SystemsTesting/colorReadTest.c functions.c -lwiringPi -lm -o ../Executables/colorRead
rotateServoTo: functions.c pins.h SystemsTesting/Centrifuge/rotateServoTo.c
	gcc -DBUILD_ROTATESERVO_MAIN SystemsTesting/Centrifuge/rotateServoTo.c functions.c -lwiringPi -o ../Executables/rotateServoTo


rotateTo_column: functions.c pins.h SystemsTesting/Column/rotateTo_column.c
	gcc -DBUILD_ROTATETO_MAIN SystemsTesting/Column/rotateTo_column.c functions.c -lwiringPi -o ../Executables/rotateTo_column
raise_lower_column: functions.c pins.h SystemsTesting/Column/raise_lower_column.c
	gcc -DBUILD_RAISELOWERC_MAIN SystemsTesting/Column/raise_lower_column.c functions.c -lwiringPi -o ../Executables/raise_lower_column
raise_lower_column_acc: functions.c pins.h SystemsTesting/Column/raise_lower_column_acc.c
	gcc -DBUILD_RAISELOWERC_MAIN SystemsTesting/Column/raise_lower_column_acc.c functions.c -lwiringPi -o ../Executables/raise_lower_column_acc


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
# waterLevelCalibrate: SystemsTesting/WaterLevel/waterLevelCalibrate.c SystemsTesting/WaterLevel/waterLevelADS1015.c
# 	gcc -DBUILD_WATERLEVELCALIBRATE_MAIN SystemsTesting/WaterLevel/waterLevelCalibrate.c SystemsTesting/WaterLevel/waterLevelADS1015.c -lwiringPi -o ../Executables/waterLevelCalibrate
pumpToHeight: SystemsTesting/pumpToHeight.c 
	gcc -DBUILD_PUMPTOHEIGHT_MAIN SystemsTesting/pumpToHeight.c -lwiringPi -lm -o ../Executables/pumpToHeight
flood_spectrometer: SystemsTesting/flood_spectrometer.c 
	gcc -DBUILD_FLOODSPEC_MAIN SystemsTesting/flood_spectrometer.c -lwiringPi -lm -o ../Executables/flood_spectrometer
waterLevelRead: #waterLevelRead.c SystemsTesting/WaterLevel/waterLevelADS1015.c SystemsTesting/WaterLevel/waterLevel.c
	gcc -DBUILD_WATERLEVELREAD_MAIN SystemsTesting/waterLevelRead.c -lwiringPi -o ../Executables/waterLevelRead
dirtSample: functions.c pins.h SystemsTesting/Automation/dirtSample.c 
	gcc -DBUILD_DSAMPLE_MAIN SystemsTesting/Automation/dirtSample.c functions.c  -lwiringPi -o ../Executables/dirtSample
chemicaltest: functions.c pins.h SystemsTesting/Automation/mixingChamberAutomation.c 
	gcc -DBUILD_DSAMPLE_MAIN SystemsTesting/Automation/mixingChamberAutomation.c functions.c -lwiringPi -o ../Executables/chemicaltest
# centrifugeSpin: functions.c pins.h SystemsTesting/Centrifuge/centrifugeSpin.c
# 	gcc -DBUILD_CENTRIFUGESPIN_MAIN SystemsTesting/Centrifuge/centrifugeSpin.c functions.c -lwiringPi -o ../Executables/centrifugeSpin
# centrifugeFluids: functions.c pins.h SystemsTesting/Automation/centrifugeFluids.c
# 	gcc -DBUILD_CENTRIFUGEFLUIDS_MAIN SystemsTesting/Automation/centrifugeFluids.c functions.c -lwiringPi -o ../Executables/centrifugeFluids
burnerController: functions.c pins.h SystemsTesting/burnerController.c
	gcc -DBUILD_BURNERCONTROLLER_MAIN SystemsTesting/burnerController.c functions.c -lwiringPi -o ../Executables/burnerController
colorRead: functions.c pins.h SystemsTesting/colorReadTest.c
	gcc -DBUILD_COLORREADTEST_MAIN SystemsTesting/colorReadTest.c functions.c -lwiringPi -lm -o ../Executables/colorRead
stream_spec_final:
	g++ Webcam/Display/stream_spectra_final.cpp -o ../Executables/stream_spec_final `pkg-config --cflags --libs opencv4`
calibrate_spectrometer:
	g++ Webcam/Display/calibrations.cpp -o ../Executables/calibrate_spectrometer -lwiringPi `pkg-config --cflags --libs opencv4`