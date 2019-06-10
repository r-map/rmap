# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations
from django.conf import settings


class Migration(migrations.Migration):

    dependencies = [
        migrations.swappable_dependency(settings.AUTH_USER_MODEL),
    ]

    operations = [
        migrations.CreateModel(
            name='Board',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
                ('name', models.CharField(help_text='station name', max_length=255)),
                ('active', models.BooleanField(default=False, help_text='Activate the board for measurements', verbose_name='Active')),
                ('slug', models.SlugField(help_text='Auto-generated from name.', unique=True)),
                ('category', models.CharField(max_length=50, choices=[(b'base', b'Raspberry base'), (b'master', b'Mega2560 master'), (b'satellite', b'Microduino core+ satellite'), (b'gsm', b'Microduino core+ GSM/GPRS with GPS'), (b'bluetooth', b'Microduino core+ with Bluetooth module')])),
            ],
            options={
                'ordering': ['slug'],
                'verbose_name': 'hardware board',
                'verbose_name_plural': 'hardware boards',
            },
        ),
        migrations.CreateModel(
            name='Sensor',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
                ('active', models.BooleanField(default=False, help_text='Activate this sensor to take measurements', verbose_name='Active')),
                ('name', models.CharField(default=b'my sensor', help_text='Descriptive text', max_length=50)),
                ('driver', models.CharField(default=b'TMP', help_text='Driver to use', max_length=4, choices=[(b'I2C', b'I2C drivers'), (b'RF24', b'RF24 Network jsonrpc'), (b'JRPC', b'INDIRECT jsonrpc over some transport')])),
                ('type', models.CharField(default=b'TMP', help_text='Type of sensor', max_length=4, choices=[(b'TMP', b'I2C TMP temperature sensor'), (b'ADT', b'I2C ADT temperature sensor'), (b'BMP', b'I2C BMP085/BMP180 pressure sensor'), (b'HIH', b'I2C HIH6100 series humidity sensor'), (b'DW1', b'I2C Davis wind direction and intensity adapter'), (b'TBR', b'I2C Tipping bucket rain gauge adapter'), (b'RF24', b'RF24 Network jsonrpc')])),
                ('i2cbus', models.PositiveIntegerField(default=1, help_text='I2C bus number (for raspberry only)', blank=True)),
                ('address', models.PositiveIntegerField(default=72, help_text='I2C ddress (decimal)')),
                ('node', models.PositiveIntegerField(default=1, help_text='RF24Network node ddress', blank=True)),
                ('timerange', models.CharField(default=b'254,0,0', help_text='Sensor metadata from rmap RFC', max_length=50)),
                ('level', models.CharField(default=b'103,2000,-,-', help_text='Sensor metadata from rmap RFC', max_length=50)),
                ('board', models.ForeignKey(to='stations.Board')),
            ],
            options={
                'ordering': ['driver'],
                'verbose_name': 'Sensor',
                'verbose_name_plural': 'Sensors',
            },
        ),
        migrations.CreateModel(
            name='StationConstantData',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
                ('active', models.BooleanField(default=True, help_text='Activate this metadata', verbose_name='Active')),
                ('btable', models.CharField(help_text='A code to define the metadata. See rmap RFC', max_length=6, choices=[(b'B01019', b'LONG STATION OR SITE NAME                                        (CCITTIA5)'), (b'B02001', b'TYPE OF STATION                                                  (CODE TABLE 2001)'), (b'B02002', b'TYPE OF INSTRUMENTATION FOR WIND MEASUREMENT                     (FLAG TABLE 2002)'), (b'B02003', b'TYPE OF MEASURING EQUIPMENT USED                                 (CODE TABLE 2003)'), (b'B02004', b'TYPE OF INSTRUMENTATION FOR EVAPORATION MEASUREMENT OR TYPE OF C (CODE TABLE 2004)'), (b'B02005', b'PRECISION OF TEMPERATURE OBSERVATION                             (K*100)'), (b'B02038', b'METHOD OF WATER TEMPERATURE AND/OR SALINITY MEASUREMENT          (CODE TABLE 2038)'), (b'B02039', b'METHOD OF WET-BULB TEMPERATURE MEASUREMENT                       (CODE TABLE 2039)'), (b'B07030', b'HEIGHT OF STATION GROUND ABOVE MEAN SEA LEVEL (SEE NOTE 3)       (m*10)'), (b'B07031', b'HEIGHT OF BAROMETER ABOVE MEAN SEA LEVEL (SEE NOTE 4)            (m*10)')])),
                ('value', models.CharField(help_text='value for associated B table', max_length=32)),
            ],
            options={
                'ordering': ['btable'],
                'verbose_name': 'Station constant metadata',
                'verbose_name_plural': 'Station constant metadata',
            },
        ),
        migrations.CreateModel(
            name='StationMetadata',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
                ('name', models.CharField(help_text='station name', max_length=255)),
                ('active', models.BooleanField(default=True, help_text='Activate the station for measurements', verbose_name='Active')),
                ('slug', models.SlugField(help_text='Auto-generated from name.', unique=True)),
                ('lat', models.FloatField(default=None, help_text='Precise Latitude of the station', verbose_name='Latitude')),
                ('lon', models.FloatField(default=None, help_text='Precise Longitude of the station', verbose_name='Longitude')),
                ('network', models.CharField(default=b'rmap', help_text='station network', max_length=50)),
                ('mqttrootpath', models.CharField(default=b'rmap', help_text='root mqtt path for publish', max_length=100)),
                ('mqttmaintpath', models.CharField(default=b'rmap', help_text='maint mqtt path for publish', max_length=100)),
                ('category', models.CharField(help_text='Category of the station', max_length=50, choices=[(b'good', b'Beautifull & Good'), (b'bad', b'Bad & Wrong'), (b'test', b'Test & Bugs'), (b'unknown', b'Unknown & Missing')])),
                ('ident', models.ForeignKey(to=settings.AUTH_USER_MODEL)),
            ],
            options={
                'ordering': ['slug'],
                'verbose_name': 'station',
                'verbose_name_plural': 'stations',
            },
        ),
        migrations.CreateModel(
            name='TransportAmqp',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
                ('active', models.BooleanField(default=False, help_text='Activate this transport for measurements', verbose_name='Active')),
                ('amqpserver', models.CharField(default=b'rmap.cc', help_text='AMQP server', max_length=50)),
                ('exchange', models.CharField(default=b'rmap', help_text='AMQP remote exchange name', max_length=50)),
                ('queue', models.CharField(default=b'rmap', help_text='AMQP local queue name', max_length=50)),
                ('amqpuser', models.CharField(default=b'', help_text='AMQP user', max_length=9, blank=True)),
                ('amqppassword', models.CharField(default=b'', help_text='AMQP password', max_length=50, blank=True)),
                ('board', models.OneToOneField(to='stations.Board')),
            ],
        ),
        migrations.CreateModel(
            name='TransportBluetooth',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
                ('active', models.BooleanField(default=False, help_text='Activate this transport for measurements', verbose_name='Active')),
                ('name', models.CharField(help_text='bluetooth name', max_length=80)),
                ('board', models.OneToOneField(to='stations.Board')),
            ],
            options={
                'ordering': ['name'],
                'verbose_name': 'bluetooth transport',
                'verbose_name_plural': 'bluetooth transport',
            },
        ),
        migrations.CreateModel(
            name='TransportMqtt',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
                ('active', models.BooleanField(default=False, help_text='Activate this transport for measurements', verbose_name='Active')),
                ('mqttsampletime', models.PositiveIntegerField(default=5, help_text='interval in seconds for publish')),
                ('mqttserver', models.CharField(default=b'mqttserver', help_text='MQTT server', max_length=50)),
                ('mqttuser', models.CharField(default=b'', help_text='MQTT user', max_length=9, blank=True)),
                ('mqttpassword', models.CharField(default=b'', help_text='MQTT password', max_length=50, blank=True)),
                ('board', models.OneToOneField(to='stations.Board')),
            ],
            options={
                'ordering': ['mqttserver'],
                'verbose_name': 'MQTT transport',
                'verbose_name_plural': 'MQTT transport',
            },
        ),
        migrations.CreateModel(
            name='TransportRF24Network',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
                ('active', models.BooleanField(default=False, help_text='Activate this transport for measurements', verbose_name='Active')),
                ('node', models.PositiveIntegerField(default=0, help_text='Node ID for RF24 Network', choices=[(0, b'RF24 Network node 0'), (1, b'RF24 Network node node 1'), (2, b'RF24 Network node node 2'), (3, b'RF24 Network node node 3'), (4, b'RF24 Network node node 4'), (5, b'RF24 Network node node 5')])),
                ('channel', models.PositiveIntegerField(default=93, help_text='Channel number for RF24', choices=[(90, b'RF24 Network node channel 90'), (91, b'RF24 Network node channel 91'), (92, b'RF24 Network node channel 92'), (93, b'RF24 Network node channel 93'), (94, b'RF24 Network node channel 94'), (95, b'RF24 Network node channel 95')])),
                ('key', models.CommaSeparatedIntegerField(blank=True, help_text='AES key', max_length=47, choices=[(b'0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15', b'preset 1'), (b'0,1,1,3,4,5,6,7,8,9,10,11,12,13,14,15', b'preset 2'), (b'0,1,2,1,4,5,6,7,8,9,10,11,12,13,14,15', b'preset 3'), (b'0,1,2,3,1,5,6,7,8,9,10,11,12,13,14,15', b'preset 4'), (b'0,1,2,3,4,1,6,7,8,9,10,11,12,13,14,15', b'preset 5'), (b'0,1,2,3,4,5,1,7,8,9,10,11,12,13,14,15', b'preset 6'), (b'0,1,2,3,4,5,6,1,8,9,10,11,12,13,14,15', b'preset 7'), (b'0,1,2,3,4,5,6,7,1,9,10,11,12,13,14,15', b'preset 8'), (b'0,1,2,3,4,5,6,7,8,1,10,11,12,13,14,15', b'preset 9')])),
                ('iv', models.CommaSeparatedIntegerField(blank=True, help_text='AES cbc iv', max_length=47, choices=[(b'0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15', b'preset 1'), (b'0,1,1,3,4,5,6,7,8,9,10,11,12,13,14,15', b'preset 2'), (b'0,1,2,1,4,5,6,7,8,9,10,11,12,13,14,15', b'preset 3'), (b'0,1,2,3,1,5,6,7,8,9,10,11,12,13,14,15', b'preset 4'), (b'0,1,2,3,4,1,6,7,8,9,10,11,12,13,14,15', b'preset 5'), (b'0,1,2,3,4,5,1,7,8,9,10,11,12,13,14,15', b'preset 6'), (b'0,1,2,3,4,5,6,1,8,9,10,11,12,13,14,15', b'preset 7'), (b'0,1,2,3,4,5,6,7,1,9,10,11,12,13,14,15', b'preset 8'), (b'0,1,2,3,4,5,6,7,8,1,10,11,12,13,14,15', b'preset 9')])),
                ('board', models.OneToOneField(to='stations.Board')),
            ],
            options={
                'ordering': ['node'],
                'verbose_name': 'RF24 Network node',
                'verbose_name_plural': 'RF24 Network nodes',
            },
        ),
        migrations.CreateModel(
            name='TransportSerial',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
                ('active', models.BooleanField(default=False, help_text='Activate this transport for measurements', verbose_name='Active')),
                ('baudrate', models.PositiveIntegerField(default=9600, help_text='Baud rate', choices=[(9600, b'9600'), (19200, b'19200'), (38400, b'38400'), (11520, b'115200')])),
                ('device', models.CharField(default=b'/dev/ttyACM0', help_text='Serial device', unique=True, max_length=30, choices=[(b'/dev/ttyACM0', b'ttyACM0'), (b'/dev/ttyUSB0', b'ttyUSB0'), (b'/dev/ttyACM1', b'ttyACM1'), (b'/dev/ttyUSB1', b'ttyUSB1'), (b'/dev/ttyACM2', b'ttyACM2'), (b'/dev/ttyUSB2', b'ttyUSB2'), (b'/dev/ttyACM3', b'ttyACM3'), (b'/dev/ttyUSB3', b'ttyUSB3'), (b'/dev/ttyACM4', b'ttyACM4'), (b'/dev/ttyUSB4', b'ttyUSB4'), (b'/dev/ttyACM5', b'ttyACM5'), (b'/dev/rfcomm0', b'rfcomm0'), (b'/dev/rfcomm1', b'rfcomm1'), (b'/dev/rfcomm2', b'rfcomm2')])),
                ('board', models.OneToOneField(to='stations.Board')),
            ],
            options={
                'ordering': ['device'],
                'verbose_name': 'serial transport',
                'verbose_name_plural': 'serial transport',
            },
        ),
        migrations.CreateModel(
            name='TransportTcpip',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
                ('active', models.BooleanField(default=False, help_text='Activate this transport for measurements', verbose_name='Active')),
                ('name', models.CharField(default=b'master', help_text='Name DSN solved (for master board only)', max_length=50, choices=[(b'master', b'master board 1'), (b'master2', b'master board 2'), (b'master3', b'master board 3'), (b'master4', b'master board 4')])),
                ('ntpserver', models.CharField(default=b'ntpserver', help_text='Network time server (NTP)', max_length=50)),
                ('board', models.OneToOneField(to='stations.Board')),
            ],
            options={
                'ordering': ['name'],
                'verbose_name': 'tcp/ip DNS resoved name',
                'verbose_name_plural': 'tcp/ip DNS resoved names',
            },
        ),
        migrations.CreateModel(
            name='UserProfile',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
                ('accepted_license', models.BooleanField(default=False, help_text='You need to accept ODBL license to provide your data', verbose_name='I accept ODBL license')),
                ('certification', models.CharField(default=b'ARPA-ER', max_length=20)),
                ('user', models.OneToOneField(to=settings.AUTH_USER_MODEL)),
            ],
        ),
        migrations.AddField(
            model_name='stationconstantdata',
            name='stationmetadata',
            field=models.ForeignKey(to='stations.StationMetadata'),
        ),
        migrations.AddField(
            model_name='board',
            name='stationmetadata',
            field=models.ForeignKey(to='stations.StationMetadata'),
        ),
    ]
