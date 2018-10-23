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
LIBS:Microduino_Base_DE-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Microduino Base DE"
Date "30 luglio 2017"
Rev "0.0"
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
NoConn ~ 7300 2950
NoConn ~ 7400 2950
NoConn ~ 7000 2950
NoConn ~ 6900 2950
NoConn ~ 6800 2950
NoConn ~ 6700 2950
NoConn ~ 6600 2950
NoConn ~ 8050 3500
NoConn ~ 8050 3600
NoConn ~ 8050 3900
NoConn ~ 8050 4000
NoConn ~ 8050 4200
$Comp
L GND #PWR01
U 1 1 58D4FFFC
P 7800 4450
F 0 "#PWR01" H 7800 4200 50  0001 C CNN
F 1 "GND" H 7800 4300 50  0000 C CNN
F 2 "" H 7800 4450 50  0000 C CNN
F 3 "" H 7800 4450 50  0000 C CNN
	1    7800 4450
	1    0    0    -1  
$EndComp
$Comp
L +5V #PWR02
U 1 1 58D50BEE
P 5450 4300
F 0 "#PWR02" H 5450 4150 50  0001 C CNN
F 1 "+5V" H 5450 4440 50  0000 C CNN
F 2 "" H 5450 4300 50  0000 C CNN
F 3 "" H 5450 4300 50  0000 C CNN
	1    5450 4300
	1    0    0    1   
$EndComp
$Comp
L +3.3V #PWR03
U 1 1 58D50CA7
P 5450 4100
F 0 "#PWR03" H 5450 3950 50  0001 C CNN
F 1 "+3.3V" H 5450 4240 50  0000 C CNN
F 2 "" H 5450 4100 50  0000 C CNN
F 3 "" H 5450 4100 50  0000 C CNN
	1    5450 4100
	1    0    0    -1  
$EndComp
Text Label 6050 4100 0    60   ~ 0
D7
Text Label 6050 4000 0    60   ~ 0
D8
Text Label 6050 3900 0    60   ~ 0
D9
Text Label 6050 3800 0    60   ~ 0
D10
Text Label 6050 3700 0    60   ~ 0
D11
Text Label 6050 3600 0    60   ~ 0
D12
Text Label 6050 3500 0    60   ~ 0
D13
NoConn ~ 6050 4100
NoConn ~ 6050 3800
NoConn ~ 6050 3700
NoConn ~ 6050 3600
NoConn ~ 6050 3500
Text Label 6600 2950 3    60   ~ 0
AREF
Text Label 6700 2950 3    60   ~ 0
A0
Text Label 6800 2950 3    60   ~ 0
A1
Text Label 6900 2950 3    60   ~ 0
A2
Text Label 7000 2950 3    60   ~ 0
A3
Text Label 7100 2950 3    60   ~ 0
SDA
Text Label 7200 2950 3    60   ~ 0
SCL
Text Label 7300 2950 3    60   ~ 0
A6
Text Label 7400 2950 3    60   ~ 0
A7
Text Label 8050 3500 2    60   ~ 0
RX0
Text Label 8050 3600 2    60   ~ 0
TX0
Text Label 8050 3700 2    60   ~ 0
D2
Text Label 8050 3800 2    60   ~ 0
D3
Text Label 8050 3900 2    60   ~ 0
D4
Text Label 8050 4000 2    60   ~ 0
D5
Text Label 8050 4100 2    60   ~ 0
D6
Text Label 8050 4200 2    60   ~ 0
RESET
Text Notes 6650 4700 0    118  ~ 24
UPIN 27
$Comp
L CONN_1x27 P3
U 1 1 58E8C7EF
P 6500 4300
F 0 "P3" H 6500 4200 50  0000 C CNN
F 1 "CONN_1x27" V 7400 4700 50  0001 C CNN
F 2 "Libreria_PCB_mia:Upin_27" H 7500 4700 50  0001 C CNN
F 3 "" H 7500 4700 50  0000 C CNN
	1    6500 4300
	1    0    0    -1  
$EndComp
Text Notes 7150 6950 0    236  Italic 47
DigitEco s.r.l.
$Comp
L CONN_01X04 P2
U 1 1 58E8CD07
P 4000 4350
F 0 "P2" H 4000 4600 50  0000 C CNN
F 1 "CONN_01X04" H 4000 4050 50  0001 C CNN
F 2 "Libreria_PCB_mia:WHURT_4pin_90°_61900411021" H 4000 4350 50  0001 C CNN
F 3 "280378-1" H 4000 4350 50  0001 C CNN
	1    4000 4350
	-1   0    0    -1  
$EndComp
Text Notes 3800 4550 2    60   ~ 0
GND
Text Notes 3800 4450 2    60   ~ 0
SDA
Text Notes 3800 4350 2    60   ~ 0
SCL
Text Notes 3800 4250 2    60   ~ 0
Vcc (3,3-5V)
NoConn ~ 8050 3700
NoConn ~ 8050 4100
NoConn ~ 8050 3800
NoConn ~ 7100 2950
NoConn ~ 7200 2950
Text Label 4550 4400 2    60   ~ 0
SDA
Text Label 4550 4300 2    60   ~ 0
SCL
$Comp
L GND #PWR04
U 1 1 58E8ED4C
P 4500 4600
F 0 "#PWR04" H 4500 4350 50  0001 C CNN
F 1 "GND" H 4500 4450 50  0000 C CNN
F 2 "" H 4500 4600 50  0000 C CNN
F 3 "" H 4500 4600 50  0000 C CNN
	1    4500 4600
	1    0    0    -1  
$EndComp
Wire Wire Line
	6050 3600 6300 3600
Wire Wire Line
	6300 3800 6050 3800
Wire Wire Line
	5800 4000 6300 4000
Wire Wire Line
	6300 4200 5700 4200
Wire Wire Line
	8050 3600 7700 3600
Wire Wire Line
	8050 4000 7700 4000
Wire Wire Line
	8050 4200 7700 4200
Wire Wire Line
	7800 4300 7800 4450
Wire Wire Line
	5250 4300 6300 4300
Wire Wire Line
	6300 4100 6050 4100
Wire Wire Line
	6300 3500 6050 3500
Wire Wire Line
	6300 3700 6050 3700
Wire Wire Line
	5250 3900 6300 3900
Wire Wire Line
	7400 3200 7400 2950
Wire Wire Line
	7200 2950 7200 3200
Wire Wire Line
	7000 2950 7000 3200
Wire Wire Line
	6800 2950 6800 3200
Wire Wire Line
	6600 2950 6600 3200
Wire Wire Line
	7300 2950 7300 3200
Wire Wire Line
	6900 2950 6900 3200
Wire Wire Line
	6700 2950 6700 3200
Wire Wire Line
	7700 3500 8050 3500
Wire Wire Line
	7700 3900 8050 3900
Wire Wire Line
	8050 3700 7700 3700
Wire Wire Line
	7100 2950 7100 3200
Wire Wire Line
	8050 4100 7700 4100
Wire Wire Line
	7700 3800 8050 3800
Wire Wire Line
	4550 4300 4200 4300
Wire Wire Line
	4200 4400 4550 4400
Wire Wire Line
	7800 4300 7700 4300
Wire Wire Line
	4950 4200 4200 4200
Connection ~ 5450 4300
Wire Wire Line
	5700 4200 5700 4100
Wire Wire Line
	5700 4100 5250 4100
Connection ~ 5450 4100
Wire Wire Line
	4200 4500 4500 4500
Wire Wire Line
	4500 4500 4500 4600
$Comp
L CONN_01X02 P1
U 1 1 58E8EE76
P 4000 3350
F 0 "P1" H 4000 3500 50  0000 C CNN
F 1 "CONN_01X02" H 4000 3150 50  0001 C CNN
F 2 "Pin_Headers:Pin_Header_Angled_1x02_Pitch2.54mm" H 4000 3350 50  0001 C CNN
F 3 "" H 4000 3350 50  0000 C CNN
	1    4000 3350
	-1   0    0    -1  
$EndComp
$Comp
L GND #PWR05
U 1 1 58E8EEDE
P 4500 3500
F 0 "#PWR05" H 4500 3250 50  0001 C CNN
F 1 "GND" H 4500 3350 50  0000 C CNN
F 2 "" H 4500 3500 50  0000 C CNN
F 3 "" H 4500 3500 50  0000 C CNN
	1    4500 3500
	1    0    0    -1  
$EndComp
Wire Wire Line
	5800 4000 5800 3300
Wire Wire Line
	5800 3300 4200 3300
Wire Wire Line
	4500 3500 4500 3400
Wire Wire Line
	4500 3400 4200 3400
Text Notes 3800 3350 2    60   ~ 0
Cortocircuitare per
Text Notes 3800 3450 2    60   ~ 0
caricare il firmware
$Comp
L GS4 GS1
U 1 1 597AF362
P 5100 4200
F 0 "GS1" H 5150 4400 50  0000 C CNN
F 1 "GS4" H 5100 3900 50  0001 C CNN
F 2 "Libreria_PCB_mia:GS4" V 5200 4100 50  0001 C CNN
F 3 "" H 5100 4200 50  0000 C CNN
	1    5100 4200
	-1   0    0    1   
$EndComp
Wire Wire Line
	5250 4000 5250 3900
$EndSCHEMATC
