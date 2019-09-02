Sample multitasking sensor handling program from cocoOS, adapted for Arduino.

Libraries needed to compile with Arduino IDE or Arduino Web Editor:

- `Time`: Download from Arduino Library Manager

- `cocoOS_5.0.1`: Download from http://www.cocoos.net/download.html, 
    unzip and move all files in `inc` and `src` to top level.
    Zip up and add to Arduino IDE as library.

Tested with Arduino Uno.

## Sample Log

```
------------------arduino_setup
display_init
os_init
event_create
tempSensor_get
tempSensor.init
gyroSensor_get
gyroSensor.init
task_create
arduino_start_timer
os_start

Gyro:		x:1	y:2	z:3

Gyro:		x:1	y:2	z:3

Gyro:		x:1	y:2	z:3

Gyro:		x:1	y:2	z:3
tempSensor_service
gyroSensor_service

Gyro:		x:1	y:2	z:3

Gyro:		x:1	y:2	z:3

Gyro:		x:1	y:2	z:3

Gyro:		x:1	y:2	z:3

Gyro:		x:1	y:2	z:3
tempSensor_service
gyroSensor_service

Gyro:		x:1	y:2	z:3

Gyro:		x:1	y:2	z:3

Gyro:		x:1	y:2	z:3

Gyro:		x:1	y:2	z:3

Gyro:		x:1	y:2	z:3
tempSensor_service
gyroSensor_service

Gyro:		x:1	y:2	z:3

Gyro:		x:1	y:2	z:3

Gyro:		x:1	y:2	z:3

Gyro:		x:1	y:2	z:3

Gyro:		x:1	y:2	z:3
tempSensor_service
gyroSensor_service

Gyro:		x:1	y:2	z:3

Gyro:		x:1	y:2	z:3

Gyro:		x:1	y:2	z:3

Gyro:		x:1	y:2	z:3

Gyro:		x:1	y:2	z:3
tempSensor_service
gyroSensor_service
New York	25 degC
Gyro:		x:1	y:2	z:3
New York	25 degC
Gyro:		x:7	y:39	z:31
New York	25 degC
Gyro:		x:7	y:39	z:31
New York	25 degC
Gyro:		x:7	y:39	z:31
New York	25 degC
Gyro:		x:7	y:39	z:31
tempSensor_service
gyroSensor_service
New York	25 degC
Gyro:		x:7	y:39	z:31
New York	25 degC
Gyro:		x:7	y:39	z:31
New York	25 degC
Gyro:		x:7	y:39	z:31
New York	25 degC
Gyro:		x:7	y:39	z:31
New York	25 degC
Gyro:		x:7	y:39	z:31
tempSensor_service
gyroSensor_service
New York	25 degC
Gyro:		x:7	y:39	z:31
New York	25 degC
Gyro:		x:7	y:39	z:31
New York	25 degC
Gyro:		x:7	y:39	z:31
New York	25 degC
Gyro:		x:7	y:39	z:31
New York	25 degC
Gyro:		x:7	y:39	z:31
tempSensor_service
gyroSensor_service
New York	25 degC
Gyro:		x:7	y:39	z:31
New York	25 degC
Gyro:		x:7	y:39	z:31
New York	25 degC
Gyro:		x:7	y:39	z:31
New York	25 degC
Gyro:		x:7	y:39	z:31
New York	25 degC
Gyro:		x:7	y:39	z:31
tempSensor_service
gyroSensor_service
New York	25 degC
Gyro:		x:7	y:39	z:31
New York	25 degC
Gyro:		x:7	y:39	z:31
New York	25 degC
Gyro:		x:7	y:39	z:31
New York	25 degC
Gyro:		x:7	y:39	z:31
New York	25 degC
Gyro:		x:7	y:39	z:31
tempSensor_service
gyroSensor_service
New York	25 degC
Gyro:		x:7	y:39	z:31
New York	25 degC
Gyro:		x:14	y:10	z:40
```
