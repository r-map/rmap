EESchema Schematic File Version 2
LIBS:Libreria_SCH_mia
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
LIBS:Microduino_Digitale_DE-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Microduino Digitale DE"
Date "30 luglio 2017"
Rev "0.0"
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
NoConn ~ 5350 3050
NoConn ~ 5450 3050
NoConn ~ 5050 3050
NoConn ~ 4950 3050
NoConn ~ 4850 3050
NoConn ~ 4750 3050
NoConn ~ 4650 3050
NoConn ~ 6100 3600
NoConn ~ 6100 3700
NoConn ~ 6100 4000
NoConn ~ 6100 4100
NoConn ~ 6100 4300
$Comp
L GND #PWR01
U 1 1 58D4FFFC
P 5850 4550
F 0 "#PWR01" H 5850 4300 50  0001 C CNN
F 1 "GND" H 5850 4400 50  0000 C CNN
F 2 "" H 5850 4550 50  0000 C CNN
F 3 "" H 5850 4550 50  0000 C CNN
	1    5850 4550
	1    0    0    -1  
$EndComp
$Comp
L +5V #PWR02
U 1 1 58D50BEE
P 3700 4400
F 0 "#PWR02" H 3700 4250 50  0001 C CNN
F 1 "+5V" H 3700 4540 50  0000 C CNN
F 2 "" H 3700 4400 50  0000 C CNN
F 3 "" H 3700 4400 50  0000 C CNN
	1    3700 4400
	1    0    0    -1  
$EndComp
$Comp
L +3.3V #PWR03
U 1 1 58D50CA7
P 3900 4300
F 0 "#PWR03" H 3900 4150 50  0001 C CNN
F 1 "+3.3V" H 3900 4440 50  0000 C CNN
F 2 "" H 3900 4300 50  0000 C CNN
F 3 "" H 3900 4300 50  0000 C CNN
	1    3900 4300
	1    0    0    -1  
$EndComp
Text Label 4100 4200 0    60   ~ 0
D7
Text Label 4100 4100 0    60   ~ 0
D8
Text Label 4100 4000 0    60   ~ 0
D9
Text Label 4100 3900 0    60   ~ 0
D10
Text Label 4100 3800 0    60   ~ 0
D11
Text Label 4100 3700 0    60   ~ 0
D12
Text Label 4100 3600 0    60   ~ 0
D13
NoConn ~ 4100 4200
NoConn ~ 4100 4100
NoConn ~ 4100 4000
NoConn ~ 4100 3900
NoConn ~ 4100 3800
NoConn ~ 4100 3700
NoConn ~ 4100 3600
Text Label 4650 3050 3    60   ~ 0
AREF
Text Label 4750 3050 3    60   ~ 0
A0
Text Label 4850 3050 3    60   ~ 0
A1
Text Label 4950 3050 3    60   ~ 0
A2
Text Label 5050 3050 3    60   ~ 0
A3
Text Label 5150 3050 3    60   ~ 0
SDA
Text Label 5250 3050 3    60   ~ 0
SCL
Text Label 5350 3050 3    60   ~ 0
A6
Text Label 5450 3050 3    60   ~ 0
A7
Wire Wire Line
	4100 3700 4350 3700
Wire Wire Line
	4350 3900 4100 3900
Wire Wire Line
	4350 4100 4100 4100
Wire Wire Line
	4350 4300 3900 4300
Wire Wire Line
	6100 3700 5750 3700
Wire Wire Line
	5750 3900 7150 3900
Wire Wire Line
	6100 4100 5750 4100
Wire Wire Line
	6100 4300 5750 4300
Wire Wire Line
	5850 4400 5850 4550
Wire Wire Line
	3700 4400 4350 4400
Wire Wire Line
	4350 4200 4100 4200
Wire Wire Line
	4350 3600 4100 3600
Wire Wire Line
	4350 3800 4100 3800
Wire Wire Line
	4350 4000 4100 4000
Wire Wire Line
	5450 3300 5450 3050
Wire Wire Line
	5250 3050 5250 3300
Wire Wire Line
	5050 3050 5050 3300
Wire Wire Line
	4850 3050 4850 3300
Wire Wire Line
	4650 3050 4650 3300
Wire Wire Line
	5350 3050 5350 3300
Wire Wire Line
	5150 3050 5150 3300
Wire Wire Line
	4950 3050 4950 3300
Wire Wire Line
	4750 3050 4750 3300
NoConn ~ 5250 3050
NoConn ~ 5150 3050
Text Label 6100 3600 2    60   ~ 0
RX0
Text Label 6100 3700 2    60   ~ 0
TX0
Text Label 6100 3800 2    60   ~ 0
D2
Text Label 6100 3900 2    60   ~ 0
D3
Text Label 6100 4000 2    60   ~ 0
D4
Text Label 6100 4100 2    60   ~ 0
D5
Text Label 6100 4200 2    60   ~ 0
D6
Text Label 6100 4300 2    60   ~ 0
RESET
Wire Wire Line
	5750 3600 6100 3600
Wire Wire Line
	5750 4000 6100 4000
Wire Wire Line
	5750 4200 6500 4200
Text Notes 4700 4800 0    118  ~ 24
UPIN 27
$Comp
L CONN_1x27 P1
U 1 1 58E8C7EF
P 4550 4400
F 0 "P1" H 4550 4300 50  0000 C CNN
F 1 "CONN_1x27" V 5450 4800 50  0001 C CNN
F 2 "Libreria_PCB_mia:Upin_27" H 5550 4800 50  0001 C CNN
F 3 "" H 5550 4800 50  0000 C CNN
	1    4550 4400
	1    0    0    -1  
$EndComp
Text Notes 7150 6950 0    236  Italic 47
DigitEco s.r.l.
$Comp
L CONN_01X04 P2
U 1 1 58E8CD07
P 7350 3950
F 0 "P2" H 7350 4200 50  0000 C CNN
F 1 "CONN_01X04" H 7350 3650 50  0001 C CNN
F 2 "Libreria_PCB_mia:WHURT_4pin_90°_61900411021" H 7350 3950 50  0001 C CNN
F 3 "280378-1" H 7350 3950 50  0001 C CNN
	1    7350 3950
	1    0    0    -1  
$EndComp
Wire Wire Line
	7150 4000 6500 4000
Wire Wire Line
	6500 4000 6500 4200
Wire Wire Line
	7150 4100 6600 4100
Wire Wire Line
	6600 4100 6600 4400
Wire Wire Line
	6600 4400 5750 4400
Connection ~ 5850 4400
Text Notes 7550 4150 0    60   ~ 0
GND
Text Notes 7550 4050 0    60   ~ 0
D6
Text Notes 7550 3950 0    60   ~ 0
D3
Text Notes 7550 3850 0    60   ~ 0
D2
Wire Wire Line
	5750 3800 7150 3800
$EndSCHEMATC
