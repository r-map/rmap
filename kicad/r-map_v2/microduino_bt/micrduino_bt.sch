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
LIBS:micrduino_bt-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Stima - Microduino bluetooth interface"
Date "2015-07-26"
Rev ""
Comp "http://rmap.cc/"
Comment1 ""
Comment2 "Released under MIT License(MIT)"
Comment3 "Author: Daniele Castellari dcast78@gmail.com"
Comment4 "This module is part of rmap project"
$EndDescr
$Comp
L ITEAD_HC-05 U2
U 1 1 55008234
P 4100 2800
F 0 "U2" H 4190 3770 60  0000 C CNN
F 1 "ITEAD_HC-05" H 4440 3690 60  0000 C CNN
F 2 "Rmap:HC-05" H 4100 2800 60  0001 C CNN
F 3 "" H 4100 2800 60  0000 C CNN
	1    4100 2800
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR01
U 1 1 55008281
P 4100 4000
F 0 "#PWR01" H 4100 4000 30  0001 C CNN
F 1 "GND" H 4100 3930 30  0001 C CNN
F 2 "" H 4100 4000 60  0000 C CNN
F 3 "" H 4100 4000 60  0000 C CNN
	1    4100 4000
	1    0    0    -1  
$EndComp
$Comp
L Microduino U1
U 1 1 550082D7
P 2250 1850
F 0 "U1" H 2250 1750 60  0000 C CNN
F 1 "Microduino" H 2150 1850 60  0000 C CNN
F 2 "Rmap:microduino" H 2250 1850 60  0001 C CNN
F 3 "" H 2250 1850 60  0000 C CNN
	1    2250 1850
	1    0    0    -1  
$EndComp
$Comp
L R R1
U 1 1 550087EF
P 1250 3000
F 0 "R1" V 1330 3000 40  0000 C CNN
F 1 "8K2" V 1257 3001 40  0000 C CNN
F 2 "w_smd_resistors:r_0805" V 1180 3000 30  0001 C CNN
F 3 "" H 1250 3000 30  0000 C CNN
	1    1250 3000
	1    0    0    -1  
$EndComp
$Comp
L R R2
U 1 1 550088C1
P 3250 3600
F 0 "R2" V 3330 3600 40  0000 C CNN
F 1 "15K" V 3257 3601 40  0000 C CNN
F 2 "w_smd_resistors:r_0805" V 3180 3600 30  0001 C CNN
F 3 "" H 3250 3600 30  0000 C CNN
	1    3250 3600
	1    0    0    -1  
$EndComp
NoConn ~ 3500 2000
NoConn ~ 3500 2150
NoConn ~ 3500 2250
NoConn ~ 3500 2600
NoConn ~ 3500 2700
NoConn ~ 3500 2800
NoConn ~ 3500 2900
NoConn ~ 3500 3050
NoConn ~ 3500 3150
NoConn ~ 3500 3300
NoConn ~ 3500 3400
NoConn ~ 3500 3500
NoConn ~ 3500 3600
NoConn ~ 4700 2100
NoConn ~ 4700 2200
NoConn ~ 4700 2400
NoConn ~ 4700 2600
NoConn ~ 4700 2800
NoConn ~ 4700 2900
NoConn ~ 4700 3000
NoConn ~ 4700 3100
NoConn ~ 4700 3300
NoConn ~ 4700 3400
$Comp
L R R3
U 1 1 5502D2EB
P 5600 3600
F 0 "R3" H 5650 3650 60  0001 C CNN
F 1 "R" H 5600 3600 60  0001 C CNN
F 2 "w_smd_resistors:r_0805" H 5600 3600 60  0001 C CNN
F 3 "" H 5600 3600 60  0001 C CNN
F 4 "R" V 5680 3600 40  0000 C CNN "Riferimento"
F 5 "150" V 5607 3601 40  0000 C CNN "Valore "
	1    5600 3600
	1    0    0    -1  
$EndComp
$Comp
L LED D1
U 1 1 5502D315
P 5600 2900
F 0 "D1" H 5650 2950 60  0001 C CNN
F 1 "LED" H 5600 2900 60  0001 C CNN
F 2 "w_smd_leds:Led_0805" H 5600 2900 60  0001 C CNN
F 3 "" H 5600 2900 60  0001 C CNN
F 4 "D" H 5600 3000 50  0000 C CNN "Riferimento"
F 5 "LED" H 5600 2800 50  0000 C CNN "Valore "
F 6 "" H 5600 2900 60  0000 C CNN "Impronta"
F 7 "" H 5600 2900 60  0000 C CNN "Specifiche"
	1    5600 2900
	0    -1   -1   0   
$EndComp
$Comp
L CONN_01X01 P3
U 1 1 5502DA6A
P 5050 3500
F 0 "P3" H 5100 3550 60  0001 C CNN
F 1 "CONN_01X01" H 5050 3500 60  0001 C CNN
F 2 "Measurement_Points:Measurement_Point_Square-SMD-Pad_Small" H 5050 3500 60  0001 C CNN
F 3 "" H 5050 3500 60  0001 C CNN
F 4 "P" H 5050 3600 50  0000 C CNN "Riferimento"
F 5 "CONN_01X01" V 5150 3500 50  0001 C CNN "Valore "
F 6 "" H 5050 3500 60  0000 C CNN "Impronta"
F 7 "" H 5050 3500 60  0000 C CNN "Specifiche"
	1    5050 3500
	1    0    0    -1  
$EndComp
Wire Wire Line
	4000 3900 4000 3750
Wire Wire Line
	4100 3750 4100 4000
Wire Wire Line
	4200 3900 4200 3750
Connection ~ 4100 3900
Wire Wire Line
	4100 1750 4100 1850
Wire Wire Line
	1150 1650 1150 3900
Connection ~ 4000 3900
Wire Wire Line
	3500 2350 3250 2350
Wire Wire Line
	3500 2450 3100 2450
Wire Wire Line
	3100 2450 3100 3200
Wire Wire Line
	3100 3200 1400 3200
Wire Wire Line
	1400 3200 1400 2250
Connection ~ 4200 3900
Wire Wire Line
	3250 3750 3250 3900
Connection ~ 3250 3900
Wire Wire Line
	1250 2150 1250 2850
Wire Wire Line
	1250 3150 1250 3300
Wire Wire Line
	1250 3300 3250 3300
Wire Wire Line
	3250 2350 3250 3450
Connection ~ 3250 3300
Wire Wire Line
	1150 3900 5600 3900
Wire Wire Line
	5600 3100 5600 3450
Wire Wire Line
	5600 3900 5600 3750
Wire Wire Line
	4700 2500 5600 2500
Wire Wire Line
	2850 1750 4100 1750
Wire Wire Line
	1450 1650 1150 1650
NoConn ~ 1450 1750
NoConn ~ 1450 1850
NoConn ~ 1450 1950
NoConn ~ 1450 2050
NoConn ~ 2850 1650
NoConn ~ 2850 1850
NoConn ~ 2850 1950
NoConn ~ 2850 2050
NoConn ~ 2850 2150
NoConn ~ 2850 2250
NoConn ~ 2850 2350
NoConn ~ 2850 2450
NoConn ~ 1750 2850
NoConn ~ 1850 2850
NoConn ~ 1950 2850
NoConn ~ 2050 2850
NoConn ~ 2150 2850
NoConn ~ 2250 2850
NoConn ~ 2350 2850
NoConn ~ 2450 2850
NoConn ~ 2550 2850
Wire Wire Line
	1250 2150 1450 2150
Wire Wire Line
	1400 2250 1450 2250
NoConn ~ 1450 2350
NoConn ~ 1450 2450
Wire Wire Line
	5600 2500 5600 2700
Wire Wire Line
	4700 3200 5300 3200
Wire Wire Line
	5300 3200 5300 2500
Connection ~ 5300 2500
NoConn ~ 4700 2700
Wire Wire Line
	4850 3500 4700 3500
Text Label 4750 3500 0    60   ~ 0
SET
Text Label 4900 3200 0    60   ~ 0
LED
Text Label 1950 2850 3    60   ~ 0
SCL
Text Label 2050 2850 3    60   ~ 0
SDA
Text Label 1450 2250 2    60   ~ 0
RX1
Text Label 1400 2150 2    60   ~ 0
TX1
Text Label 2850 1950 0    60   ~ 0
CONFIG
Text Label 2850 2450 0    60   ~ 0
SCK
Text Label 2850 2350 0    60   ~ 0
MISO
Text Label 2850 2250 0    60   ~ 0
MOSI
Text Label 2850 2150 0    60   ~ 0
CS_NRF24
Text Label 2850 2050 0    60   ~ 0
CE_NRF24
Text Label 1250 2150 2    60   ~ 0
INT_28J60
$EndSCHEMATC
