EESchema Schematic File Version 2
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
LIBS:mics-2714
EELAYER 26 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "NO2 breakout board"
Date ""
Rev ""
Comp "raspibo"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L R R?
U 1 1 5890EBCA
P 2800 2500
F 0 "R?" H 2870 2546 50  0000 L CNN
F 1 "R" H 2870 2455 50  0000 L CNN
F 2 "" V -70 0   50  0001 C CNN
F 3 "" H 0   0   50  0001 C CNN
	1    2800 2500
	1    0    0    -1  
$EndComp
$Comp
L MICS-2714 U?
U 1 1 5890F0F2
P 5850 2750
F 0 "U?" H 5850 3647 60  0000 C CNN
F 1 "MICS-2714" H 5850 3541 60  0000 C CNN
F 2 "" V -350 250 60  0001 C CNN
F 3 "" V -350 250 60  0001 C CNN
	1    5850 2750
	1    0    0    -1  
$EndComp
$Comp
L +5V #PWR?
U 1 1 5890F170
P 6700 1900
F 0 "#PWR?" H -300 -650 50  0001 C CNN
F 1 "+5V" H 6715 2073 50  0000 C CNN
F 2 "" H -300 -500 50  0001 C CNN
F 3 "" H -300 -500 50  0001 C CNN
	1    6700 1900
	1    0    0    -1  
$EndComp
Wire Wire Line
	6700 1900 6700 2400
Wire Wire Line
	6700 2400 6350 2400
Wire Wire Line
	6350 2100 6700 2100
Connection ~ 6700 2100
$Comp
L R R?
U 1 1 5890F2A0
P 4950 3000
F 0 "R?" H 5020 3046 50  0000 L CNN
F 1 "R" H 5020 2955 50  0000 L CNN
F 2 "" V -120 350 50  0001 C CNN
F 3 "" H -50 350 50  0001 C CNN
	1    4950 3000
	1    0    0    -1  
$EndComp
$Comp
L R R?
U 1 1 5890F2F4
P 4550 3000
F 0 "R?" H 4620 3046 50  0000 L CNN
F 1 "R" H 4620 2955 50  0000 L CNN
F 2 "" V -220 0   50  0001 C CNN
F 3 "" H -150 0   50  0001 C CNN
	1    4550 3000
	1    0    0    -1  
$EndComp
$Comp
L 2N7002 Q?
U 1 1 5890F594
P 4450 3650
F 0 "Q?" H 4641 3696 50  0000 L CNN
F 1 "2N7002" H 4641 3605 50  0000 L CNN
F 2 "TO_SOT_Packages_SMD:SOT-23" H 150 75  50  0001 L CIN
F 3 "" H -50 150 50  0001 L CNN
	1    4450 3650
	1    0    0    -1  
$EndComp
$EndSCHEMATC
