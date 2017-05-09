EESchema Schematic File Version 2
LIBS:Stima_AirQuality_Connector-rescue
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:microduino
LIBS:Stima_AirQuality_Connector-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L +5V #PWR01
U 1 1 589CCCE0
P 5950 2450
F 0 "#PWR01" H -50 -250 50  0001 C CNN
F 1 "+5V" H 5965 2623 50  0000 C CNN
F 2 "" H -50 -100 50  0001 C CNN
F 3 "" H -50 -100 50  0001 C CNN
	1    5950 2450
	1    0    0    -1  
$EndComp
Wire Wire Line
	5950 2450 5950 2550
Wire Wire Line
	5950 2550 5800 2550
$Comp
L GND #PWR02
U 1 1 589CCCFC
P 4150 2650
F 0 "#PWR02" H 0   -150 50  0001 C CNN
F 1 "GND" H 4155 2477 50  0000 C CNN
F 2 "" H 0   100 50  0001 C CNN
F 3 "" H 0   100 50  0001 C CNN
	1    4150 2650
	1    0    0    -1  
$EndComp
Wire Wire Line
	4150 2650 4150 2550
Wire Wire Line
	4150 2550 4400 2550
$Comp
L +3.3V #PWR03
U 1 1 589CCD1E
P 6150 2450
F 0 "#PWR03" H -50 -600 50  0001 C CNN
F 1 "+3.3V" H 6165 2623 50  0000 C CNN
F 2 "" H -50 -450 50  0001 C CNN
F 3 "" H -50 -450 50  0001 C CNN
	1    6150 2450
	1    0    0    -1  
$EndComp
Wire Wire Line
	6150 2450 6150 2650
Wire Wire Line
	6150 2650 5800 2650
$Comp
L CONN_01X05 P3
U 1 1 589CCD3F
P 7650 3200
F 0 "P3" H 7728 3241 50  0000 L CNN
F 1 "CONN_01X05" H 7728 3150 50  0000 L CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x05_Pitch2.54mm" H 1400 -250 50  0001 C CNN
F 3 "" H 1400 -250 50  0001 C CNN
	1    7650 3200
	0    -1   -1   0   
$EndComp
Text GLabel 7550 3650 3    60   Input ~ 0
SCL
Text GLabel 7650 3650 3    60   Input ~ 0
SDA
$Comp
L GND #PWR04
U 1 1 589CCDD1
P 7750 3850
F 0 "#PWR04" H 7800 3900 50  0001 C CNN
F 1 "GND" H 7755 3677 50  0000 C CNN
F 2 "" H 0   -150 50  0001 C CNN
F 3 "" H 0   -150 50  0001 C CNN
	1    7750 3850
	1    0    0    -1  
$EndComp
$Comp
L +3.3V #PWR05
U 1 1 589CCE09
P 8050 3350
F 0 "#PWR05" H 8100 3400 50  0001 C CNN
F 1 "+3.3V" H 8065 3523 50  0000 C CNN
F 2 "" H 150 -300 50  0001 C CNN
F 3 "" H 150 -300 50  0001 C CNN
	1    8050 3350
	1    0    0    -1  
$EndComp
Wire Wire Line
	8050 3350 8050 3500
Wire Wire Line
	8050 3500 7850 3500
Wire Wire Line
	7850 3500 7850 3400
Wire Wire Line
	7750 3400 7750 3850
Wire Wire Line
	7650 3650 7650 3400
Wire Wire Line
	7550 3400 7550 3650
$Comp
L +5V #PWR06
U 1 1 589CCE6E
P 7250 3350
F 0 "#PWR06" H 7300 3400 50  0001 C CNN
F 1 "+5V" H 7265 3523 50  0000 C CNN
F 2 "" H -150 -300 50  0001 C CNN
F 3 "" H -150 -300 50  0001 C CNN
	1    7250 3350
	1    0    0    -1  
$EndComp
Wire Wire Line
	7250 3350 7250 3500
Wire Wire Line
	7250 3500 7450 3500
Wire Wire Line
	7450 3500 7450 3400
$Comp
L CONN_01X05 P1
U 1 1 589CCFAB
P 4100 5350
F 0 "P1" H 4019 4925 50  0000 C CNN
F 1 "CONN_01X05" H 4019 5016 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x05_Pitch2.54mm" H -1350 100 50  0001 C CNN
F 3 "" H -1350 100 50  0001 C CNN
	1    4100 5350
	-1   0    0    1   
$EndComp
$Comp
L CONN_01X05 P2
U 1 1 589CD012
P 4100 5850
F 0 "P2" H 4019 5425 50  0000 C CNN
F 1 "CONN_01X05" H 4019 5516 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x05_Pitch2.54mm" H 0   -50 50  0001 C CNN
F 3 "" H 0   -50 50  0001 C CNN
	1    4100 5850
	-1   0    0    1   
$EndComp
Text GLabel 4650 5050 1    60   Input ~ 0
NO2
Text GLabel 4800 5050 1    60   Input ~ 0
CO
Text GLabel 5250 5050 1    60   Input ~ 0
SCALE_2
Text GLabel 5400 5050 1    60   Input ~ 0
SCALE_1
Text GLabel 5550 5050 1    60   Input ~ 0
PWM
Wire Wire Line
	4650 5050 4650 5150
Wire Wire Line
	4650 5150 4300 5150
Wire Wire Line
	4300 5250 4800 5250
Wire Wire Line
	4800 5250 4800 5050
$Comp
L +5V #PWR07
U 1 1 589CD236
P 5700 5570
F 0 "#PWR07" H 5750 5620 50  0001 C CNN
F 1 "+5V" H 5715 5743 50  0000 C CNN
F 2 "" H -200 -430 50  0001 C CNN
F 3 "" H -200 -430 50  0001 C CNN
	1    5700 5570
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR08
U 1 1 589CD287
P 5700 6150
F 0 "#PWR08" H 5750 6200 50  0001 C CNN
F 1 "GND" H 5705 5977 50  0000 C CNN
F 2 "" H 1400 200 50  0001 C CNN
F 3 "" H 1400 200 50  0001 C CNN
	1    5700 6150
	1    0    0    -1  
$EndComp
Wire Wire Line
	4300 5950 5700 5950
Wire Wire Line
	5700 5950 5700 6150
Wire Wire Line
	4300 6050 5700 6050
Connection ~ 5700 6050
$Comp
L CONN_01X04 P4
U 1 1 589CD4E0
P 7650 4850
F 0 "P4" V 7650 5150 50  0000 R CNN
F 1 "CONN_01X04" V 7750 5050 50  0000 R CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x04_Pitch2.54mm" H 5750 -250 50  0001 C CNN
F 3 "" H 5750 -250 50  0001 C CNN
	1    7650 4850
	0    -1   -1   0   
$EndComp
Text GLabel 7500 5250 3    60   Input ~ 0
RX
Text GLabel 7600 5250 3    60   Input ~ 0
TX
$Comp
L GND #PWR09
U 1 1 589CD795
P 7700 5250
F 0 "#PWR09" H 7750 5300 50  0001 C CNN
F 1 "GND" H 7705 5077 50  0000 C CNN
F 2 "" H 0   0   50  0001 C CNN
F 3 "" H 0   0   50  0001 C CNN
	1    7700 5250
	1    0    0    -1  
$EndComp
$Comp
L +5V #PWR010
U 1 1 589CD7C2
P 7950 5050
F 0 "#PWR010" H 8000 5100 50  0001 C CNN
F 1 "+5V" H 7965 5223 50  0000 C CNN
F 2 "" H 100 -200 50  0001 C CNN
F 3 "" H 100 -200 50  0001 C CNN
	1    7950 5050
	1    0    0    -1  
$EndComp
Wire Wire Line
	7800 5050 7800 5150
Wire Wire Line
	7800 5150 7950 5150
Wire Wire Line
	7950 5150 7950 5050
Wire Wire Line
	7700 5050 7700 5250
Wire Wire Line
	7600 5050 7600 5250
Wire Wire Line
	7500 5250 7500 5050
Text GLabel 6000 2950 2    60   Input ~ 0
SCALE_2
Text GLabel 6000 2850 2    60   Input ~ 0
SCALE_1
Text GLabel 6000 2750 2    60   Input ~ 0
PWM
Wire Wire Line
	6000 2750 5800 2750
Wire Wire Line
	5800 2850 6000 2850
Wire Wire Line
	6000 2950 5800 2950
Text GLabel 5400 3900 3    60   Input ~ 0
NO2
Text GLabel 5300 3900 3    60   Input ~ 0
CO
Wire Wire Line
	5300 3900 5300 3750
Wire Wire Line
	5400 3750 5400 3900
NoConn ~ 5800 3250
NoConn ~ 5800 3350
NoConn ~ 4700 3750
NoConn ~ 4800 3750
NoConn ~ 5100 3750
NoConn ~ 5200 3750
Text GLabel 4900 3900 3    60   Input ~ 0
SCL
Text GLabel 5000 3900 3    60   Input ~ 0
SDA
Wire Wire Line
	5000 3900 5000 3750
Wire Wire Line
	4900 3750 4900 3900
NoConn ~ 4400 2650
NoConn ~ 4400 2750
NoConn ~ 4400 2850
NoConn ~ 4400 2950
$Comp
L Microduino U1
U 1 1 589CF128
P 5200 2750
F 0 "U1" H 5100 3197 60  0000 C CNN
F 1 "Microduino" H 5100 3091 60  0000 C CNN
F 2 "" H -50 100 60  0000 C CNN
F 3 "" H -50 100 60  0000 C CNN
	1    5200 2750
	1    0    0    -1  
$EndComp
Text Label 4450 5450 0    60   ~ 0
SCALE1
Text Label 4450 5350 0    60   ~ 0
SCALE2
NoConn ~ 5800 3050
NoConn ~ 5800 3150
Text GLabel 5500 3900 3    60   Input ~ 0
VREF
Wire Wire Line
	5500 3900 5500 3750
Wire Wire Line
	4300 5350 5250 5350
Wire Wire Line
	5250 5350 5250 5050
Wire Wire Line
	5400 5050 5400 5450
Wire Wire Line
	5400 5450 4300 5450
Wire Wire Line
	4300 5550 5550 5550
Wire Wire Line
	5550 5550 5550 5050
Wire Wire Line
	4300 5650 5700 5650
Wire Wire Line
	5700 5650 5700 5570
Text GLabel 4510 5850 2    60   Input ~ 0
VREF
Wire Wire Line
	4510 5850 4300 5850
NoConn ~ 4300 5750
$EndSCHEMATC
