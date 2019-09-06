EESchema Schematic File Version 4
LIBS:rfm95
LIBS:microduino
LIBS:power
LIBS:microduino_lora-cache
EELAYER 26 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Microduino Lora shield"
Date ""
Rev "1.0"
Comp "raspibo.org"
Comment1 "Released under MIT License(MIT)"
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L microduino_lora-rescue:Microduino-RESCUE-microduino_lora U1
U 1 1 59F8DF1A
P 4100 3000
F 0 "U1" H 4000 3447 60  0000 C CNN
F 1 "Microduino" H 4000 3341 60  0000 C CNN
F 2 "lib:microduino" H 4100 3000 60  0001 C CNN
F 3 "" H 4100 3000 60  0000 C CNN
	1    4100 3000
	1    0    0    -1  
$EndComp
NoConn ~ 3300 3000
NoConn ~ 3300 3100
NoConn ~ 3300 3200
NoConn ~ 3300 3300
NoConn ~ 4100 4000
NoConn ~ 3300 3500
NoConn ~ 3300 3600
NoConn ~ 3800 4000
NoConn ~ 3900 4000
NoConn ~ 4700 3100
NoConn ~ 4200 4000
NoConn ~ 4300 4000
NoConn ~ 4400 4000
NoConn ~ 4700 3200
NoConn ~ 3300 2900
NoConn ~ 4700 3000
NoConn ~ 4700 2800
NoConn ~ 6500 3400
NoConn ~ 7500 3300
NoConn ~ 7500 3200
Wire Wire Line
	4900 2900 4700 2900
Wire Wire Line
	6100 3100 6500 3100
Wire Wire Line
	6100 3000 6500 3000
Wire Wire Line
	6100 2900 6500 2900
Wire Wire Line
	6100 3200 6500 3200
Text GLabel 6100 3100 0    60   Input ~ 0
SCK
Text GLabel 6100 3000 0    60   Input ~ 0
MOSI
Text GLabel 6100 2900 0    60   Input ~ 0
MISO
Text GLabel 6100 3200 0    60   Input ~ 0
NSS
Text GLabel 6100 3300 0    60   Input ~ 0
RST
Text GLabel 4000 4500 3    60   Input ~ 0
RST
Text GLabel 4950 3600 2    60   Input ~ 0
SCK
Text GLabel 4950 3500 2    60   Input ~ 0
MISO
Text GLabel 4950 3400 2    60   Input ~ 0
MOSI
Text GLabel 4950 3300 2    60   Input ~ 0
NSS
Wire Wire Line
	4950 3300 4700 3300
Wire Wire Line
	4700 3400 4950 3400
Wire Wire Line
	4950 3500 4700 3500
Wire Wire Line
	4700 3600 4950 3600
$Comp
L power:+3V3 #PWR01
U 1 1 5A231BBA
P 4900 2850
F 0 "#PWR01" H 4900 2700 50  0001 C CNN
F 1 "+3V3" H 4915 3023 50  0000 C CNN
F 2 "" H 4900 2850 50  0001 C CNN
F 3 "" H 4900 2850 50  0001 C CNN
	1    4900 2850
	1    0    0    -1  
$EndComp
Wire Wire Line
	6100 3300 6500 3300
$Comp
L power:GND #PWR02
U 1 1 5A23263B
P 6450 3550
F 0 "#PWR02" H 6450 3300 50  0001 C CNN
F 1 "GND" H 6455 3377 50  0000 C CNN
F 2 "" H 6450 3550 50  0001 C CNN
F 3 "" H 6450 3550 50  0001 C CNN
	1    6450 3550
	1    0    0    -1  
$EndComp
Wire Wire Line
	6450 3550 6450 3500
Wire Wire Line
	6450 3500 6500 3500
$Comp
L power:GND #PWR03
U 1 1 5A23280D
P 7750 3800
F 0 "#PWR03" H 7750 3550 50  0001 C CNN
F 1 "GND" H 7755 3627 50  0000 C CNN
F 2 "" H 7750 3800 50  0001 C CNN
F 3 "" H 7750 3800 50  0001 C CNN
	1    7750 3800
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR04
U 1 1 5A2329E5
P 6350 2800
F 0 "#PWR04" H 6350 2550 50  0001 C CNN
F 1 "GND" V 6355 2672 50  0000 R CNN
F 2 "" H 6350 2800 50  0001 C CNN
F 3 "" H 6350 2800 50  0001 C CNN
	1    6350 2800
	0    1    1    0   
$EndComp
Wire Wire Line
	6350 2800 6500 2800
$Comp
L power:+3V3 #PWR05
U 1 1 5A232C05
P 7600 3100
F 0 "#PWR05" H 7600 2950 50  0001 C CNN
F 1 "+3V3" V 7615 3228 50  0000 L CNN
F 2 "" H 7600 3100 50  0001 C CNN
F 3 "" H 7600 3100 50  0001 C CNN
	1    7600 3100
	0    1    1    0   
$EndComp
Wire Wire Line
	7600 3100 7500 3100
Text GLabel 7600 3000 2    60   Input ~ 0
TxRxDONE
Wire Wire Line
	7600 2900 7500 2900
Text GLabel 7600 2900 2    60   Input ~ 0
RxTimeOut
Wire Wire Line
	7600 3000 7500 3000
$Comp
L lora:RFM95 U2
U 1 1 5A231F4D
P 7000 3150
F 0 "U2" H 7000 3747 60  0000 C CNN
F 1 "RFM95" H 7000 3641 60  0000 C CNN
F 2 "lib:RFM95" H 7000 3150 60  0001 C CNN
F 3 "" H 7000 3150 60  0000 C CNN
	1    7000 3150
	1    0    0    -1  
$EndComp
Text GLabel 7600 2800 2    60   Input ~ 0
FSKTimeOut
Wire Wire Line
	7600 2800 7500 2800
Text GLabel 3600 4350 3    60   Input ~ 0
RxTimeOut
Text GLabel 3150 3400 0    60   Input ~ 0
TxRxDONE
Wire Wire Line
	3700 4000 3700 4150
Text GLabel 3700 4350 3    60   Input ~ 0
FSKTimeOut
$Comp
L power:GND #PWR06
U 1 1 5A232045
P 3300 2800
F 0 "#PWR06" H 3300 2550 50  0001 C CNN
F 1 "GND" V 3305 2672 50  0000 R CNN
F 2 "" H 3300 2800 50  0001 C CNN
F 3 "" H 3300 2800 50  0001 C CNN
	1    3300 2800
	0    1    1    0   
$EndComp
$Comp
L conn_Coaxial:Conn_Coaxial J1
U 1 1 5A24EEFE
P 7750 3500
F 0 "J1" H 7550 3400 50  0000 L CNN
F 1 "Conn_Coaxial" H 7150 3200 50  0000 L CNN
F 2 "lib:SMA_EDGE" H 7750 3500 50  0001 C CNN
F 3 "" H 7750 3500 50  0001 C CNN
	1    7750 3500
	1    0    0    -1  
$EndComp
Wire Wire Line
	7600 3500 7500 3500
Wire Wire Line
	7750 3800 7750 3750
Wire Wire Line
	7750 3750 7900 3750
Wire Wire Line
	7900 3750 7900 3400
Wire Wire Line
	7900 3400 7500 3400
Connection ~ 7750 3750
Wire Wire Line
	7750 3750 7750 3700
$Comp
L power:PWR_FLAG #FLG01
U 1 1 5A26E5C0
P 5100 2900
F 0 "#FLG01" H 5100 2975 50  0001 C CNN
F 1 "PWR_FLAG" V 5100 3028 50  0000 L CNN
F 2 "" H 5100 2900 50  0001 C CNN
F 3 "" H 5100 2900 50  0001 C CNN
	1    5100 2900
	0    1    1    0   
$EndComp
Wire Wire Line
	5100 2900 4900 2900
Connection ~ 4900 2900
Wire Wire Line
	4900 2850 4900 2900
Wire Wire Line
	3150 3400 3300 3400
Wire Wire Line
	3600 4150 3600 4000
Wire Wire Line
	4000 4500 4000 4000
$Comp
L conn_Coaxial:Conn_01x01 J2
U 1 1 5A3D8281
P 3400 4150
F 0 "J2" H 3400 4050 50  0000 C CNN
F 1 "Conn_01x01" H 3700 4150 50  0000 C CNN
F 2 "Measurement_Points:Measurement_Point_Round-SMD-Pad_Small" H 3400 4150 50  0001 C CNN
F 3 "~" H 3400 4150 50  0001 C CNN
	1    3400 4150
	-1   0    0    1   
$EndComp
$Comp
L conn_Coaxial:Conn_01x01 J3
U 1 1 5A3D831B
P 3400 4300
F 0 "J3" H 3400 4400 50  0000 C CNN
F 1 "Conn_01x01" H 3700 4300 50  0000 C CNN
F 2 "Measurement_Points:Measurement_Point_Round-SMD-Pad_Small" H 3400 4300 50  0001 C CNN
F 3 "~" H 3400 4300 50  0001 C CNN
	1    3400 4300
	-1   0    0    1   
$EndComp
$Comp
L conn_Coaxial:Conn_01x01 J4
U 1 1 5A3D83DF
P 3900 4150
F 0 "J4" H 3850 4250 50  0000 L CNN
F 1 "Conn_01x01" H 3980 4101 50  0000 L CNN
F 2 "Measurement_Points:Measurement_Point_Round-SMD-Pad_Small" H 3900 4150 50  0001 C CNN
F 3 "~" H 3900 4150 50  0001 C CNN
	1    3900 4150
	1    0    0    -1  
$EndComp
$Comp
L conn_Coaxial:Conn_01x01 J5
U 1 1 5A3D8810
P 3900 4300
F 0 "J5" H 3850 4200 50  0000 L CNN
F 1 "Conn_01x01" H 3980 4251 50  0000 L CNN
F 2 "Measurement_Points:Measurement_Point_Round-SMD-Pad_Small" H 3900 4300 50  0001 C CNN
F 3 "~" H 3900 4300 50  0001 C CNN
	1    3900 4300
	1    0    0    -1  
$EndComp
Wire Wire Line
	3600 4300 3600 4350
Wire Wire Line
	3700 4350 3700 4300
$EndSCHEMATC
