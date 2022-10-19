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
LIBS:Microduino_Supporto_DE-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Microduino SUPPORTO DE"
Date "24 dicembre 2017"
Rev "0.0"
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
NoConn ~ 4400 2650
NoConn ~ 4500 2650
NoConn ~ 4100 2650
NoConn ~ 4000 2650
NoConn ~ 3900 2650
NoConn ~ 3800 2650
NoConn ~ 3700 2650
NoConn ~ 5150 3200
NoConn ~ 5150 3300
NoConn ~ 5150 3600
NoConn ~ 5150 3700
NoConn ~ 5150 3900
$Comp
L GND #PWR01
U 1 1 58D4FFFC
P 4900 4150
F 0 "#PWR01" H 4900 3900 50  0001 C CNN
F 1 "GND" H 4900 4000 50  0000 C CNN
F 2 "" H 4900 4150 50  0000 C CNN
F 3 "" H 4900 4150 50  0000 C CNN
	1    4900 4150
	1    0    0    -1  
$EndComp
Text Label 3150 3800 0    60   ~ 0
D7
Text Label 3150 3700 0    60   ~ 0
D8
Text Label 3150 3600 0    60   ~ 0
D9
Text Label 3150 3500 0    60   ~ 0
D10
Text Label 3150 3400 0    60   ~ 0
D11
Text Label 3150 3300 0    60   ~ 0
D12
Text Label 3150 3200 0    60   ~ 0
D13
NoConn ~ 3150 3800
NoConn ~ 3150 3600
NoConn ~ 3150 3500
NoConn ~ 3150 3400
NoConn ~ 3150 3300
NoConn ~ 3150 3200
Text Label 3700 2650 3    60   ~ 0
AREF
Text Label 3800 2650 3    60   ~ 0
A0
Text Label 3900 2650 3    60   ~ 0
A1
Text Label 4000 2650 3    60   ~ 0
A2
Text Label 4100 2650 3    60   ~ 0
A3
Text Label 4200 2650 3    60   ~ 0
SDA
Text Label 4300 2650 3    60   ~ 0
SCL
Text Label 4400 2650 3    60   ~ 0
A6
Text Label 4500 2650 3    60   ~ 0
A7
Text Label 5150 3200 2    60   ~ 0
RX0
Text Label 5150 3300 2    60   ~ 0
TX0
Text Label 5150 3400 2    60   ~ 0
D2
Text Label 5150 3500 2    60   ~ 0
D3
Text Label 5150 3600 2    60   ~ 0
D4
Text Label 5150 3700 2    60   ~ 0
D5
Text Label 5150 3800 2    60   ~ 0
D6
Text Label 5150 3900 2    60   ~ 0
RESET
Text Notes 3750 4400 0    118  ~ 24
UPIN 27
$Comp
L CONN_1x27 P1
U 1 1 58E8C7EF
P 3600 4000
F 0 "P1" H 3600 3900 50  0000 C CNN
F 1 "CONN_1x27" V 4500 4400 50  0000 C CNN
F 2 "Libreria_PCB_mia:Upin_27" H 4600 4400 50  0001 C CNN
F 3 "" H 4600 4400 50  0000 C CNN
	1    3600 4000
	1    0    0    -1  
$EndComp
Text Notes 7150 6950 0    236  Italic 47
DigitEco s.r.l.
NoConn ~ 5150 3400
NoConn ~ 5150 3800
NoConn ~ 5150 3500
NoConn ~ 4200 2650
NoConn ~ 4300 2650
Wire Wire Line
	3150 3300 3400 3300
Wire Wire Line
	3400 3500 3150 3500
Wire Wire Line
	5150 3300 4800 3300
Wire Wire Line
	5150 3700 4800 3700
Wire Wire Line
	5150 3900 4800 3900
Wire Wire Line
	4900 4000 4900 4150
Wire Wire Line
	3400 3800 3150 3800
Wire Wire Line
	3400 3200 3150 3200
Wire Wire Line
	3400 3400 3150 3400
Wire Wire Line
	3400 3600 3150 3600
Wire Wire Line
	4500 2900 4500 2650
Wire Wire Line
	4300 2650 4300 2900
Wire Wire Line
	4100 2650 4100 2900
Wire Wire Line
	3900 2650 3900 2900
Wire Wire Line
	3700 2650 3700 2900
Wire Wire Line
	4400 2650 4400 2900
Wire Wire Line
	4000 2650 4000 2900
Wire Wire Line
	3800 2650 3800 2900
Wire Wire Line
	4800 3200 5150 3200
Wire Wire Line
	4800 3600 5150 3600
Wire Wire Line
	5150 3400 4800 3400
Wire Wire Line
	4200 2650 4200 2900
Wire Wire Line
	5150 3800 4800 3800
Wire Wire Line
	4800 3500 5150 3500
Wire Wire Line
	4900 4000 4800 4000
Wire Wire Line
	3400 3900 2950 3900
$Comp
L +3.3V #PWR02
U 1 1 58D50CA7
P 2950 3900
F 0 "#PWR02" H 2950 3750 50  0001 C CNN
F 1 "+3.3V" H 2950 4040 50  0000 C CNN
F 2 "" H 2950 3900 50  0000 C CNN
F 3 "" H 2950 3900 50  0000 C CNN
	1    2950 3900
	1    0    0    -1  
$EndComp
$Comp
L +5V #PWR03
U 1 1 58D50BEE
P 2750 4000
F 0 "#PWR03" H 2750 3850 50  0001 C CNN
F 1 "+5V" H 2750 4140 50  0000 C CNN
F 2 "" H 2750 4000 50  0000 C CNN
F 3 "" H 2750 4000 50  0000 C CNN
	1    2750 4000
	-1   0    0    -1  
$EndComp
Wire Wire Line
	3150 3700 3400 3700
Wire Wire Line
	3400 4000 2750 4000
NoConn ~ 3150 3700
$Comp
L +3.3V #PWR?
U 1 1 5A3F86DC
P 7600 3300
F 0 "#PWR?" H 7600 3150 50  0001 C CNN
F 1 "+3.3V" H 7600 3440 50  0000 C CNN
F 2 "" H 7600 3300 50  0001 C CNN
F 3 "" H 7600 3300 50  0001 C CNN
	1    7600 3300
	1    0    0    -1  
$EndComp
$Comp
L +5V #PWR?
U 1 1 5A3F86F2
P 8250 3300
F 0 "#PWR?" H 8250 3150 50  0001 C CNN
F 1 "+5V" H 8250 3440 50  0000 C CNN
F 2 "" H 8250 3300 50  0001 C CNN
F 3 "" H 8250 3300 50  0001 C CNN
	1    8250 3300
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 5A3F8708
P 8900 3700
F 0 "#PWR?" H 8900 3450 50  0001 C CNN
F 1 "GND" H 8900 3550 50  0000 C CNN
F 2 "" H 8900 3700 50  0001 C CNN
F 3 "" H 8900 3700 50  0001 C CNN
	1    8900 3700
	1    0    0    -1  
$EndComp
$Comp
L PWR_FLAG #FLG?
U 1 1 5A3F871E
P 8900 3300
F 0 "#FLG?" H 8900 3375 50  0001 C CNN
F 1 "PWR_FLAG" H 8900 3450 50  0000 C CNN
F 2 "" H 8900 3300 50  0001 C CNN
F 3 "" H 8900 3300 50  0001 C CNN
	1    8900 3300
	1    0    0    -1  
$EndComp
$Comp
L PWR_FLAG #FLG?
U 1 1 5A3F8734
P 8250 3700
F 0 "#FLG?" H 8250 3775 50  0001 C CNN
F 1 "PWR_FLAG" H 8250 3850 50  0000 C CNN
F 2 "" H 8250 3700 50  0001 C CNN
F 3 "" H 8250 3700 50  0001 C CNN
	1    8250 3700
	-1   0    0    1   
$EndComp
$Comp
L PWR_FLAG #FLG?
U 1 1 5A3F874A
P 7600 3700
F 0 "#FLG?" H 7600 3775 50  0001 C CNN
F 1 "PWR_FLAG" H 7600 3850 50  0000 C CNN
F 2 "" H 7600 3700 50  0001 C CNN
F 3 "" H 7600 3700 50  0001 C CNN
	1    7600 3700
	-1   0    0    1   
$EndComp
Wire Wire Line
	8900 3300 8900 3700
Wire Wire Line
	8250 3300 8250 3700
Wire Wire Line
	7600 3300 7600 3700
$EndSCHEMATC
