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
Date "02 agosto 2017"
Rev "0.0"
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
NoConn ~ 5700 2650
NoConn ~ 5800 2650
NoConn ~ 5400 2650
NoConn ~ 5300 2650
NoConn ~ 5200 2650
NoConn ~ 5100 2650
NoConn ~ 5000 2650
NoConn ~ 6450 3200
NoConn ~ 6450 3300
NoConn ~ 6450 3600
NoConn ~ 6450 3700
NoConn ~ 6450 3900
$Comp
L GND #PWR01
U 1 1 58D4FFFC
P 6200 4150
F 0 "#PWR01" H 6200 3900 50  0001 C CNN
F 1 "GND" H 6200 4000 50  0000 C CNN
F 2 "" H 6200 4150 50  0000 C CNN
F 3 "" H 6200 4150 50  0000 C CNN
	1    6200 4150
	1    0    0    -1  
$EndComp
Text Label 4450 3800 0    60   ~ 0
D7
Text Label 4450 3700 0    60   ~ 0
D8
Text Label 4450 3600 0    60   ~ 0
D9
Text Label 4450 3500 0    60   ~ 0
D10
Text Label 4450 3400 0    60   ~ 0
D11
Text Label 4450 3300 0    60   ~ 0
D12
Text Label 4450 3200 0    60   ~ 0
D13
NoConn ~ 4450 3800
NoConn ~ 4450 3600
NoConn ~ 4450 3500
NoConn ~ 4450 3400
NoConn ~ 4450 3300
NoConn ~ 4450 3200
Text Label 5000 2650 3    60   ~ 0
AREF
Text Label 5100 2650 3    60   ~ 0
A0
Text Label 5200 2650 3    60   ~ 0
A1
Text Label 5300 2650 3    60   ~ 0
A2
Text Label 5400 2650 3    60   ~ 0
A3
Text Label 5500 2650 3    60   ~ 0
SDA
Text Label 5600 2650 3    60   ~ 0
SCL
Text Label 5700 2650 3    60   ~ 0
A6
Text Label 5800 2650 3    60   ~ 0
A7
Text Label 6450 3200 2    60   ~ 0
RX0
Text Label 6450 3300 2    60   ~ 0
TX0
Text Label 6450 3400 2    60   ~ 0
D2
Text Label 6450 3500 2    60   ~ 0
D3
Text Label 6450 3600 2    60   ~ 0
D4
Text Label 6450 3700 2    60   ~ 0
D5
Text Label 6450 3800 2    60   ~ 0
D6
Text Label 6450 3900 2    60   ~ 0
RESET
Text Notes 5050 4400 0    118  ~ 24
UPIN 27
$Comp
L CONN_1x27 P1
U 1 1 58E8C7EF
P 4900 4000
F 0 "P1" H 4900 3900 50  0000 C CNN
F 1 "CONN_1x27" V 5800 4400 50  0000 C CNN
F 2 "Libreria_PCB_mia:Upin_27" H 5900 4400 50  0001 C CNN
F 3 "" H 5900 4400 50  0000 C CNN
	1    4900 4000
	1    0    0    -1  
$EndComp
Text Notes 7150 6950 0    236  Italic 47
DigitEco s.r.l.
NoConn ~ 6450 3400
NoConn ~ 6450 3800
NoConn ~ 6450 3500
NoConn ~ 5500 2650
NoConn ~ 5600 2650
Wire Wire Line
	4450 3300 4700 3300
Wire Wire Line
	4700 3500 4450 3500
Wire Wire Line
	6450 3300 6100 3300
Wire Wire Line
	6450 3700 6100 3700
Wire Wire Line
	6450 3900 6100 3900
Wire Wire Line
	6200 4000 6200 4150
Wire Wire Line
	4700 3800 4450 3800
Wire Wire Line
	4700 3200 4450 3200
Wire Wire Line
	4700 3400 4450 3400
Wire Wire Line
	4700 3600 4450 3600
Wire Wire Line
	5800 2900 5800 2650
Wire Wire Line
	5600 2650 5600 2900
Wire Wire Line
	5400 2650 5400 2900
Wire Wire Line
	5200 2650 5200 2900
Wire Wire Line
	5000 2650 5000 2900
Wire Wire Line
	5700 2650 5700 2900
Wire Wire Line
	5300 2650 5300 2900
Wire Wire Line
	5100 2650 5100 2900
Wire Wire Line
	6100 3200 6450 3200
Wire Wire Line
	6100 3600 6450 3600
Wire Wire Line
	6450 3400 6100 3400
Wire Wire Line
	5500 2650 5500 2900
Wire Wire Line
	6450 3800 6100 3800
Wire Wire Line
	6100 3500 6450 3500
Wire Wire Line
	6200 4000 6100 4000
Wire Wire Line
	4700 3900 4250 3900
$Comp
L +3.3V #PWR02
U 1 1 58D50CA7
P 4250 3900
F 0 "#PWR02" H 4250 3750 50  0001 C CNN
F 1 "+3.3V" H 4250 4040 50  0000 C CNN
F 2 "" H 4250 3900 50  0000 C CNN
F 3 "" H 4250 3900 50  0000 C CNN
	1    4250 3900
	1    0    0    -1  
$EndComp
$Comp
L +5V #PWR03
U 1 1 58D50BEE
P 4050 4000
F 0 "#PWR03" H 4050 3850 50  0001 C CNN
F 1 "+5V" H 4050 4140 50  0000 C CNN
F 2 "" H 4050 4000 50  0000 C CNN
F 3 "" H 4050 4000 50  0000 C CNN
	1    4050 4000
	-1   0    0    -1  
$EndComp
Wire Wire Line
	4450 3700 4700 3700
Wire Wire Line
	4700 4000 4050 4000
NoConn ~ 4450 3700
$EndSCHEMATC
