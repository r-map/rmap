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
LIBS:Microduino_Core+_DE-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Microduino Core+ DE"
Date "30 luglio 2017"
Rev "0.0"
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L GND #PWR01
U 1 1 58D4FFFC
P 9050 3600
F 0 "#PWR01" H 9050 3350 50  0001 C CNN
F 1 "GND" H 9050 3450 50  0000 C CNN
F 2 "" H 9050 3600 50  0000 C CNN
F 3 "" H 9050 3600 50  0000 C CNN
	1    9050 3600
	1    0    0    -1  
$EndComp
$Comp
L +5V #PWR02
U 1 1 58D50BEE
P 7100 3450
F 0 "#PWR02" H 7100 3300 50  0001 C CNN
F 1 "+5V" H 7100 3590 50  0000 C CNN
F 2 "" H 7100 3450 50  0000 C CNN
F 3 "" H 7100 3450 50  0000 C CNN
	1    7100 3450
	1    0    0    1   
$EndComp
$Comp
L +3.3V #PWR03
U 1 1 58D50CA7
P 7100 3350
F 0 "#PWR03" H 7100 3200 50  0001 C CNN
F 1 "+3.3V" H 7100 3490 50  0000 C CNN
F 2 "" H 7100 3350 50  0000 C CNN
F 3 "" H 7100 3350 50  0000 C CNN
	1    7100 3350
	1    0    0    -1  
$EndComp
Text Label 7300 3250 0    60   ~ 0
D7
Text Label 7300 3150 0    60   ~ 0
D8
Text Label 7300 3050 0    60   ~ 0
D9
Text Label 7300 2950 0    60   ~ 0
D10
Text Label 7300 2850 0    60   ~ 0
D11
Text Label 7300 2750 0    60   ~ 0
D12
Text Label 7300 2650 0    60   ~ 0
D13
Text Label 7850 2100 3    60   ~ 0
AREF
Text Label 7950 2100 3    60   ~ 0
A0
Text Label 8050 2100 3    60   ~ 0
A1
Text Label 8150 2100 3    60   ~ 0
A2
Text Label 8250 2100 3    60   ~ 0
A3
Text Label 8350 2100 3    60   ~ 0
SDA
Text Label 8450 2100 3    60   ~ 0
SCL
Text Label 8550 2100 3    60   ~ 0
A6
Text Label 8650 2100 3    60   ~ 0
A7
Text Label 9400 2650 2    60   ~ 0
D0(RX0)
Text Label 9400 2750 2    60   ~ 0
D1(TX0)
Text Label 9400 2850 2    60   ~ 0
D2(RX1)
Text Label 9400 2950 2    60   ~ 0
D3(TX1)
Text Label 9400 3050 2    60   ~ 0
D4
Text Label 9400 3150 2    60   ~ 0
D5
Text Label 9400 3250 2    60   ~ 0
D6
Text Label 9400 3350 2    60   ~ 0
RESET
Text Notes 7900 3850 0    118  ~ 24
UPIN 27
$Comp
L CONN_1x27 P1
U 1 1 58E8C7EF
P 7750 3450
F 0 "P1" H 7750 3350 50  0000 C CNN
F 1 "CONN_1x27" V 8650 3850 50  0001 C CNN
F 2 "Libreria_PCB_mia:Upin_27" H 8750 3850 50  0001 C CNN
F 3 "" H 8750 3850 50  0000 C CNN
	1    7750 3450
	1    0    0    -1  
$EndComp
Text Notes 7150 6950 0    236  Italic 47
DigitEco s.r.l.
$Comp
L CP C2
U 1 1 594FDD53
P 3900 1650
F 0 "C2" H 3925 1750 50  0000 L CNN
F 1 "10uF" H 3925 1550 50  0000 L CNN
F 2 "Capacitors_Tantalum_SMD:CP_Tantalum_Case-B_EIA-3528-21_Reflow" H 3938 1500 50  0001 C CNN
F 3 "" H 3900 1650 50  0000 C CNN
	1    3900 1650
	1    0    0    -1  
$EndComp
$Comp
L C C3
U 1 1 594FDE58
P 4300 1650
F 0 "C3" H 4325 1750 50  0000 L CNN
F 1 "100nF" H 4325 1550 50  0000 L CNN
F 2 "Capacitors_SMD:C_0805" H 4338 1500 50  0001 C CNN
F 3 "" H 4300 1650 50  0000 C CNN
	1    4300 1650
	1    0    0    -1  
$EndComp
$Comp
L R R1
U 1 1 59502FA7
P 2400 1850
F 0 "R1" V 2480 1850 50  0000 C CNN
F 1 "4k7" V 2400 1850 50  0000 C CNN
F 2 "Resistors_SMD:R_0805" V 2330 1850 50  0001 C CNN
F 3 "" H 2400 1850 50  0000 C CNN
	1    2400 1850
	-1   0    0    1   
$EndComp
$Comp
L GND #PWR04
U 1 1 59503460
P 3300 6300
F 0 "#PWR04" H 3300 6050 50  0001 C CNN
F 1 "GND" H 3300 6150 50  0000 C CNN
F 2 "" H 3300 6300 50  0000 C CNN
F 3 "" H 3300 6300 50  0000 C CNN
	1    3300 6300
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR05
U 1 1 59504B94
P 3900 1900
F 0 "#PWR05" H 3900 1650 50  0001 C CNN
F 1 "GND" H 3900 1750 50  0000 C CNN
F 2 "" H 3900 1900 50  0000 C CNN
F 3 "" H 3900 1900 50  0000 C CNN
	1    3900 1900
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR06
U 1 1 59504BBA
P 4300 1900
F 0 "#PWR06" H 4300 1650 50  0001 C CNN
F 1 "GND" H 4300 1750 50  0000 C CNN
F 2 "" H 4300 1900 50  0000 C CNN
F 3 "" H 4300 1900 50  0000 C CNN
	1    4300 1900
	1    0    0    -1  
$EndComp
$Comp
L ATMEGA644PA-A IC1
U 1 1 59759D4C
P 3600 4100
F 0 "IC1" H 2750 5980 50  0000 L BNN
F 1 "ATMEGA644PA-A" H 4000 2150 50  0000 L BNN
F 2 "Housings_QFP:TQFP-44_10x10mm_Pitch0.8mm" H 3600 4100 50  0001 C CIN
F 3 "" H 3600 4100 50  0000 C CNN
	1    3600 4100
	1    0    0    -1  
$EndComp
Text Label 4900 2400 0    60   ~ 0
A7
Text Label 4900 2500 0    60   ~ 0
A6
Text Label 4900 2600 0    60   ~ 0
A5
Text Label 4900 2700 0    60   ~ 0
A4
Text Label 4900 2800 0    60   ~ 0
A3
Text Label 4900 2900 0    60   ~ 0
A2
Text Label 4900 3000 0    60   ~ 0
A1
Text Label 4900 3100 0    60   ~ 0
A0
Text Label 4900 4000 0    60   ~ 0
D13
Text Label 4900 3900 0    60   ~ 0
D12
Text Label 4900 3800 0    60   ~ 0
D11
Text Label 4900 3700 0    60   ~ 0
D10
Text Label 4900 3600 0    60   ~ 0
D7
Text Label 4900 3500 0    60   ~ 0
D6
Text Label 4900 3400 0    60   ~ 0
D5
Text Label 4900 3300 0    60   ~ 0
D4
NoConn ~ 8450 2150
Text Label 4900 4200 0    60   ~ 0
SCL
Text Label 4900 4300 0    60   ~ 0
SDA
$Comp
L CONN_02X05 P2
U 1 1 5975B925
P 8350 5100
F 0 "P2" H 8350 5400 50  0000 C CNN
F 1 "CONN_02X05" H 8350 4800 50  0001 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_2x05_Pitch2.54mm" H 8350 3900 50  0001 C CNN
F 3 "" H 8350 3900 50  0000 C CNN
	1    8350 5100
	1    0    0    -1  
$EndComp
Text Label 7800 4900 2    60   ~ 0
D14
Text Label 7800 5000 2    60   ~ 0
D16(TDI)
Text Label 4900 4900 0    60   ~ 0
D14
Text Label 4900 4800 0    60   ~ 0
D15
Text Label 8900 4900 0    60   ~ 0
D15
Text Label 4900 4700 0    60   ~ 0
D16(TDI)
Text Label 4900 4600 0    60   ~ 0
D17(TDO)
Text Label 8900 5000 0    60   ~ 0
D17(TDO)
Text Label 4900 4500 0    60   ~ 0
D18(TMS)
Text Label 7800 5100 2    60   ~ 0
D18(TMS)
Text Label 4900 4400 0    60   ~ 0
D19(TCK)
Text Label 8900 5100 0    60   ~ 0
D19(TCK)
Text Label 4900 5800 0    60   ~ 0
D23
Text Label 8900 5300 0    60   ~ 0
D23
Text Label 4900 5700 0    60   ~ 0
D8
Text Label 4900 5600 0    60   ~ 0
D9
Text Label 7800 5300 2    60   ~ 0
D22
Text Label 4900 5500 0    60   ~ 0
D22
Text Label 4900 5400 0    60   ~ 0
D3(TX1)
Text Label 4900 5300 0    60   ~ 0
D2(RX1)
Text Label 4900 5200 0    60   ~ 0
D1(TX0)
Text Label 4900 5100 0    60   ~ 0
D0(RX0)
Text Label 8900 5200 0    60   ~ 0
A5
Text Label 7800 5200 2    60   ~ 0
A4
Text Label 2100 2400 2    60   ~ 0
RESET
$Comp
L +3.3V #PWR07
U 1 1 5975E8D0
P 5300 1200
F 0 "#PWR07" H 5300 1050 50  0001 C CNN
F 1 "+3.3V" H 5300 1340 50  0000 C CNN
F 2 "" H 5300 1200 50  0000 C CNN
F 3 "" H 5300 1200 50  0000 C CNN
	1    5300 1200
	1    0    0    -1  
$EndComp
Text Label 2100 3600 2    60   ~ 0
AREF
Wire Wire Line
	7300 2750 7550 2750
Wire Wire Line
	7550 2950 7300 2950
Wire Wire Line
	7550 3350 7100 3350
Wire Wire Line
	9400 3150 8950 3150
Wire Wire Line
	9400 3350 8950 3350
Wire Wire Line
	9050 3450 9050 3600
Wire Wire Line
	7100 3450 7550 3450
Wire Wire Line
	7550 3250 7300 3250
Wire Wire Line
	7550 2650 7300 2650
Wire Wire Line
	7550 2850 7300 2850
Wire Wire Line
	7550 3050 7300 3050
Wire Wire Line
	8650 2350 8650 2100
Wire Wire Line
	8450 2100 8450 2350
Wire Wire Line
	8250 2100 8250 2350
Wire Wire Line
	8050 2100 8050 2350
Wire Wire Line
	7850 2100 7850 2350
Wire Wire Line
	8550 2100 8550 2350
Wire Wire Line
	8150 2100 8150 2350
Wire Wire Line
	7950 2100 7950 2350
Wire Wire Line
	8950 3050 9400 3050
Wire Wire Line
	8350 2100 8350 2350
Wire Wire Line
	9050 3450 8950 3450
Wire Wire Line
	7300 3150 7550 3150
Wire Wire Line
	3300 6100 3300 6300
Wire Wire Line
	4300 1800 4300 1900
Wire Wire Line
	3900 1800 3900 1900
Wire Wire Line
	4900 2400 4600 2400
Wire Wire Line
	4600 2500 4900 2500
Wire Wire Line
	4900 2600 4600 2600
Wire Wire Line
	4600 2700 4900 2700
Wire Wire Line
	4900 2800 4600 2800
Wire Wire Line
	4900 2900 4600 2900
Wire Wire Line
	4600 3000 4900 3000
Wire Wire Line
	4900 3100 4600 3100
Wire Wire Line
	4900 3300 4600 3300
Wire Wire Line
	4600 3400 4900 3400
Wire Wire Line
	4900 3500 4600 3500
Wire Wire Line
	4600 3600 4900 3600
Wire Wire Line
	4900 3700 4600 3700
Wire Wire Line
	4600 3800 4900 3800
Wire Wire Line
	4900 3900 4600 3900
Wire Wire Line
	4600 4000 4900 4000
Wire Wire Line
	9400 3250 8950 3250
Wire Wire Line
	9400 2650 8950 2650
Wire Wire Line
	8950 2750 9400 2750
Wire Wire Line
	9400 2850 8950 2850
Wire Wire Line
	8950 2950 9400 2950
Wire Wire Line
	4900 4200 4600 4200
Wire Wire Line
	4900 4300 4600 4300
Wire Wire Line
	4600 4400 4900 4400
Wire Wire Line
	4900 4500 4600 4500
Wire Wire Line
	4600 4600 4900 4600
Wire Wire Line
	4900 4700 4600 4700
Wire Wire Line
	4600 4800 4900 4800
Wire Wire Line
	4900 4900 4600 4900
Wire Wire Line
	4600 5100 4900 5100
Wire Wire Line
	4900 5200 4600 5200
Wire Wire Line
	4600 5300 4900 5300
Wire Wire Line
	4900 5400 4600 5400
Wire Wire Line
	4600 5500 4900 5500
Wire Wire Line
	4900 5600 4600 5600
Wire Wire Line
	4600 5700 4900 5700
Wire Wire Line
	4900 5800 4600 5800
Wire Wire Line
	7800 4900 8100 4900
Wire Wire Line
	8100 5000 7800 5000
Wire Wire Line
	7800 5100 8100 5100
Wire Wire Line
	8100 5200 7800 5200
Wire Wire Line
	7800 5300 8100 5300
Wire Wire Line
	8600 4900 8900 4900
Wire Wire Line
	8900 5000 8600 5000
Wire Wire Line
	8600 5100 8900 5100
Wire Wire Line
	8900 5200 8600 5200
Wire Wire Line
	8600 5300 8900 5300
Wire Wire Line
	3300 6200 3600 6200
Wire Wire Line
	3600 6200 3600 6100
Connection ~ 3300 6200
Wire Wire Line
	3500 6200 3500 6100
Connection ~ 3500 6200
Wire Wire Line
	3400 6200 3400 6100
Connection ~ 3400 6200
Wire Wire Line
	2400 1300 4700 1300
Wire Wire Line
	2400 1300 2400 1700
Wire Wire Line
	3300 2100 3300 1300
Connection ~ 3300 1300
Wire Wire Line
	3400 2100 3400 1300
Connection ~ 3400 1300
Wire Wire Line
	3500 2100 3500 1300
Connection ~ 3500 1300
Wire Wire Line
	3700 2100 3700 1300
Connection ~ 3700 1300
Wire Wire Line
	2100 2400 2600 2400
Wire Wire Line
	2400 2400 2400 2000
Connection ~ 2400 2400
Wire Wire Line
	2100 3600 2600 3600
Wire Wire Line
	4300 1500 4300 1300
Connection ~ 4300 1300
Wire Wire Line
	3900 1500 3900 1300
Connection ~ 3900 1300
$Comp
L C C1
U 1 1 5975F8C7
P 3000 1650
F 0 "C1" H 3025 1750 50  0000 L CNN
F 1 "100nF" H 3025 1550 50  0000 L CNN
F 2 "Libreria_PCB_mia:C_0805_315°" H 3038 1500 50  0001 C CNN
F 3 "" H 3000 1650 50  0000 C CNN
	1    3000 1650
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR08
U 1 1 5975F8CD
P 3000 1900
F 0 "#PWR08" H 3000 1650 50  0001 C CNN
F 1 "GND" H 3000 1750 50  0000 C CNN
F 2 "" H 3000 1900 50  0000 C CNN
F 3 "" H 3000 1900 50  0000 C CNN
	1    3000 1900
	1    0    0    -1  
$EndComp
Wire Wire Line
	3000 1800 3000 1900
Wire Wire Line
	3000 1500 3000 1300
$Comp
L C C4
U 1 1 5975F99E
P 2400 3950
F 0 "C4" H 2425 4050 50  0000 L CNN
F 1 "100nF" H 2425 3850 50  0000 L CNN
F 2 "Capacitors_SMD:C_0805" H 2438 3800 50  0001 C CNN
F 3 "" H 2400 3950 50  0000 C CNN
	1    2400 3950
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR09
U 1 1 5975F9A4
P 2400 4200
F 0 "#PWR09" H 2400 3950 50  0001 C CNN
F 1 "GND" H 2400 4050 50  0000 C CNN
F 2 "" H 2400 4200 50  0000 C CNN
F 3 "" H 2400 4200 50  0000 C CNN
	1    2400 4200
	1    0    0    -1  
$EndComp
Wire Wire Line
	2400 4100 2400 4200
Wire Wire Line
	2400 3800 2400 3600
$Comp
L Crystal_GND2 Y1
U 1 1 59760148
P 2300 3000
F 0 "Y1" H 2300 3225 50  0000 C CNN
F 1 "8MHz" H 2300 3150 50  0000 C CNN
F 2 "Libreria_PCB_mia:Resonator_SMD_muRata_CST-3pin_3.2x1.3mm" H 2300 3000 50  0001 C CNN
F 3 "" H 2300 3000 50  0000 C CNN
	1    2300 3000
	0    1    1    0   
$EndComp
Wire Wire Line
	2600 2800 2300 2800
Wire Wire Line
	2300 2800 2300 2850
Wire Wire Line
	2300 3150 2300 3200
Wire Wire Line
	2300 3200 2600 3200
$Comp
L GND #PWR010
U 1 1 597602A9
P 1850 3100
F 0 "#PWR010" H 1850 2850 50  0001 C CNN
F 1 "GND" H 1850 2950 50  0000 C CNN
F 2 "" H 1850 3100 50  0000 C CNN
F 3 "" H 1850 3100 50  0000 C CNN
	1    1850 3100
	1    0    0    -1  
$EndComp
Wire Wire Line
	1850 3100 1850 3000
Wire Wire Line
	1850 3000 2100 3000
Connection ~ 3000 1300
Connection ~ 2400 3600
$Comp
L +5V #PWR011
U 1 1 597CF798
P 5300 1400
F 0 "#PWR011" H 5300 1250 50  0001 C CNN
F 1 "+5V" H 5300 1540 50  0000 C CNN
F 2 "" H 5300 1400 50  0000 C CNN
F 3 "" H 5300 1400 50  0000 C CNN
	1    5300 1400
	-1   0    0    1   
$EndComp
$Comp
L GS3 GS1
U 1 1 597CFCA5
P 4850 1300
F 0 "GS1" H 4850 1550 50  0000 C CNN
F 1 "GS3" H 4900 1101 50  0001 C CNN
F 2 "Connectors:GS3" V 4938 1226 50  0001 C CNN
F 3 "" H 4850 1300 50  0000 C CNN
	1    4850 1300
	-1   0    0    1   
$EndComp
Wire Wire Line
	5000 1200 5300 1200
Wire Wire Line
	5300 1400 5000 1400
Text Notes 5850 1400 0    98   ~ 20
PER ATMEGA 644 COLLEGARE +3V3 E Y1 DA 8MHz\nPER ATMEGA 1284 COLLEGARE +5V E Y1 DA 16MHz
$EndSCHEMATC
