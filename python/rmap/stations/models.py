from imagekit.models import ImageSpecField
from imagekit.models import ProcessedImageField
from imagekit.processors import ResizeToFill, Transpose, SmartResize, ResizeToFit
from  django import VERSION as djversion

#from django.core import urlresolvers
from django.db import models
from django.contrib.auth.models import User
from django.utils.translation import ugettext_lazy

from django.db.models import Q

from  django import VERSION as djversion
from rmap.utils import nint
#from leaflet.forms.fields import PointField
from django.core.exceptions import ValidationError
from django.core.validators import validate_comma_separated_integer_list
from rmap.registration.signals import user_activated
import random

try:
    import dballe
    dballepresent=True
except ImportError:
    print("dballe utilities disabled")
    dballepresent=False

try:
    from django.contrib.gis.geos import Point
    gdalpresent=True
except:
    print("gdal utilities disabled")
    gdalpresent=False

if ((djversion[0] == 1 and djversion[1] >= 3) or 
    djversion[0] > 1):

    from django.db.models import signals

    class DeletingImageField(ProcessedImageField):
        """
        ProcessedImageField subclass that deletes the refernced file when the model object
        itself is deleted.
        
        WARNING: Be careful using this class - it can cause data loss! This class
        makes at attempt to see if the file's referenced elsewhere, but it can get
        it wrong in any number of cases.
        """
        def contribute_to_class(self, cls, name):
            super(DeletingImageField, self).contribute_to_class(cls, name)
            signals.post_delete.connect(self.delete_file, sender=cls)
        
        def delete_file(self, instance, sender, **kwargs):
            file = getattr(instance, self.attname)
            # If no other object of this type references the file,
            # and it's not the default value for future objects,
            # delete it from the backend.
            
            if file and file.name != self.default and \
                    not sender._default_manager.filter(**{self.name: file.name}):
                file.delete(save=False)
            elif file:
                # Otherwise, just close the file, so it doesn't tie up resources.
                file.close()

else:
    DeletingImageField=ProcessedImageField

    
def toint(level):
    ilevel=[]
    for ele in level.split(","):
        try:
            iele=int(ele)
        except:
            iele=None
        ilevel.append(iele)
    return ilevel


if ((djversion[0] == 1 and djversion[1] >= 3) or 
    djversion[0] > 1):

    from django.db.models import signals

    class DeletingFileField(models.FileField):
        """
        FileField subclass that deletes the refernced file when the model object
        itself is deleted.
        
        WARNING: Be careful using this class - it can cause data loss! This class
        makes at attempt to see if the file's referenced elsewhere, but it can get
        it wrong in any number of cases.
        """
        def contribute_to_class(self, cls, name):
            super(DeletingFileField, self).contribute_to_class(cls, name)
            signals.post_delete.connect(self.delete_file, sender=cls)
        
        def delete_file(self, instance, sender, **kwargs):
            file = getattr(instance, self.attname)
            # If no other object of this type references the file,
            # and it's not the default value for future objects,
            # delete it from the backend.
            
            if file and file.name != self.default and \
                    not sender._default_manager.filter(**{self.name: file.name}):
                file.delete(save=False)
            elif file:
                # Otherwise, just close the file, so it doesn't tie up resources.
                file.close()

else:
    DeletingFileField=models.FileField



class SensorManager(models.Manager):
    def get_by_natural_key(self, name, board):
        #print "SensorManager: ",name,board
        return self.get(name=name, board=Board.objects.get_by_natural_key(board[0],board[1]))

class Sensor(models.Model):
    """Sensor driver metadata."""

    objects = SensorManager()

    SENSOR_DRIVER_CHOICES = (
        ('CAN',  'Cyphal over CAN-BUS'),
        ('I2C',  'I2C drivers'),
        ('RF24',  'RF24 Network jsonrpc'),
        ('SERI',  'SERIAL drivers over serial port'),
        ('JRPC',  'INDIRECT jsonrpc over some transport'),
    )

    active = models.BooleanField(ugettext_lazy("Active"),default=False,null=False,blank=False,help_text=ugettext_lazy("Activate this sensor to take measurements"))
    name = models.CharField(max_length=50,default="my sensor",blank=False,help_text=ugettext_lazy("Descriptive text"))

    driver = models.CharField(max_length=4,default="I2C",null=False,blank=False,choices=SENSOR_DRIVER_CHOICES,help_text=ugettext_lazy("Driver to use"))

    type = models.ForeignKey('SensorType',null=False,blank=False,help_text=ugettext_lazy("Type of sensor"),on_delete=models.CASCADE)

    i2cbus=models.PositiveIntegerField(default=None,null=True,blank=True,help_text=ugettext_lazy("I2C bus number (for raspberry only)"))
    address=models.PositiveIntegerField(default=None,null=True,blank=True,help_text=ugettext_lazy("I2C address (decimal)"))
    node=models.PositiveIntegerField(default=None,null=True,blank=True,help_text=ugettext_lazy("RF24Network node ddress"))

    timerange = models.CharField(max_length=50,unique=False,default="254,0,0",null=False,blank=False,help_text=ugettext_lazy("Sensor metadata from rmap RFC"))
    level = models.CharField(max_length=50,unique=False,default="103,2000,-,-",null=False,blank=False,help_text=ugettext_lazy("Sensor metadata from rmap RFC"))
    
    board = models.ForeignKey('Board',on_delete=models.CASCADE)


    def clean(self):
        # check sensor datalevel with roothpath
        if not self.active: return
        if not self.board.active: return
        if not self.board.stationmetadata.active: return

        if self.board.stationmetadata.mqttrootpath != self.type.datalevel:
            raise ValidationError(ugettext_lazy('Station and sensor have different data level; change mqttrootpath or active sensors.'))

    def dynamic_timerange(self):

        try:
            if self.board.transportmqtt.active:
                #board have mqtttransport active; use it
                return self.timerange.format(P2=self.board.transportmqtt.mqttsampletime)
        except:
            pass

        #board do not have mqtttransport active; search for other board in station with mqtttransport
        mystation=self.board.stationmetadata
        for board in mystation.board_set.all():
            # try to get sampletime: from mqtt transport of one board of the station
            # we use the first found
            if (hasattr(board, 'transportmqtt')):
                if (board.transportmqtt.active) :
                    return self.timerange.format(P2=board.transportmqtt.mqttsampletime)

        print ("dynamic_timerange warning: return timerange with default 900 sec. if not static defined")
        return self.timerange.format(P2=900)
                
    def underscored_timerange(self):
        return self.dynamic_timerange().replace(',','_')

    def underscored_level(self):
        return self.level.replace(',','_')

    def describe_level(self):
        if dballepresent:
            return dballe.describe_level(*toint(self.level))
        else:
            return self.level

    def describe_timerange(self):
        if dballepresent:
            return dballe.describe_trange(*toint(self.dynamic_timerange()))
        else:
            return self.dynamic_timerange

    def natural_key(self):
        #print "natural key sensor"
        #print self,self.name, self.board.natural_key()
        return (self.name, self.board.natural_key())

    natural_key.dependencies = ['stations.board','stations.sensortype']

#    def validate_unique(self, *args, **kwargs):
#        super(Sensor, self).validate_unique(*args, **kwargs)
#        if not self.id:
#            if self.__class__.objects.filter(name=name,board=board).exists():
#                raise ValidationError(
#                    {
#                        NON_FIELD_ERRORS: [
#                            'Sensor with same name already exists for this board.',
#                        ],
#                    }
#                )

    class Meta:
        ordering = ['driver']
        verbose_name = 'Sensor' 
        verbose_name_plural = 'Sensors' 
        # TODO unique will be unique for each timerange, level, station !
        unique_together = (('name', 'board'),)

    def __str__(self):
        #return u'%s-%s-%s-%s-%d-%s-%s-%s' % (self.name,self.active,self.driver,self.type,self.address,self.timerange,self.level,self.board)
        return '%s-%s-%s' % (self.name,self.active,self.driver)



class SensorTypeManager(models.Manager):
    def get_by_natural_key(self, type):
        #print "SensorTypeManager: ",type
        return self.get(type=type)

class SensorType(models.Model):
    """Sensor type metadata."""

    objects = SensorTypeManager()

#    SENSOR_TYPE_CHOICES = (
#        ('TMP',  'I2C TMP temperature sensor'),
#        ('ADT',  'I2C ADT temperature sensor'),
#        ('BMP',  'I2C BMP085/BMP180 pressure sensor'),
#        ('HIH',  'I2C HIH6100 series humidity sensor'),
#        ('DW1',  'I2C Davis/Inspeed/Windsonic wind direction and intensity adapter'),
#        ('TBR',  'I2C Tipping bucket rain gauge adapter'),
#        ('RF24', 'RF24 Network jsonrpc'),
#
#        ('STH',  'I2C TH module, one shot mode'),
#        ('ITH',  'I2C TH module, report mode, istantaneous values'),
#        ('NTH',  'I2C TH module, report mode, minimum values'),
#        ('MTH',  'I2C TH module, report mode, mean values'),
#        ('XTH',  'I2C TH module, report mode, maximum values'),
#        ('SSD',  'I2C SDS011 module, one shot mode'),
#    )

    SENSOR_DATA_LEVEL = (
        ('sample',  'Sensor provide data at Level I (sample)'),
        ('report',  'Sensor provide data at Level II (report)'),
    )


    active = models.BooleanField(ugettext_lazy("Active"),default=True,null=False,blank=False,help_text=ugettext_lazy("Activate this sensor to take measurements"))
    name = models.CharField(max_length=100,default="my sensor type",blank=False,help_text=ugettext_lazy("Descriptive text"))

    type = models.CharField(unique=True,max_length=4,default="TMP",null=False,blank=False,help_text=ugettext_lazy("Type of sensor"))

    datalevel = models.CharField(max_length=10,unique=False,default="sample",null=False,choices=SENSOR_DATA_LEVEL,blank=False,help_text=ugettext_lazy("Data Level as defined by WMO (Sensor metadata from rmap RFC)"))
    
    bcodes = models.ManyToManyField('Bcode',blank=False,help_text=ugettext_lazy("Bcode variable definition"))
    
    def natural_key(self):
        #print "natural key sensor type"
        #print self,self.type, self.board.natural_key()
        return (self.type,)

    natural_key.dependencies = ['stations.bcode']
    
    class Meta:
        ordering = ['type']
        verbose_name = 'Sensor Type' 
        verbose_name_plural = 'Sensors Type' 
        #unique_together = (('name', 'type'),)

    def __str__(self):
        return '%s-%s-%s' % (self.name,self.active,self.type)



class BcodeManager(models.Manager):
    def get_by_natural_key(self, bcode):
        #print "BcodeManager: ",bcode
        return self.get(bcode=bcode)

class Bcode(models.Model):
    """Variable definition."""

    objects = BcodeManager()

    bcode = models.CharField(unique=True,max_length=6,default="B00000",blank=False,help_text=ugettext_lazy("Bcode as defined in dballe btable"))
    description = models.CharField(max_length=100,default="Undefined",blank=False,help_text=ugettext_lazy("Descriptive text"))
    unit = models.CharField(max_length=20,default="Undefined",blank=False,help_text=ugettext_lazy("Units of measure"))
    offset=models.FloatField(default=0.,null=False,blank=False,help_text=ugettext_lazy("Offset coeficent to convert units"))
    scale=models.FloatField(default=1.,null=False,blank=False,help_text=ugettext_lazy("Scale coeficent to convert units"))
    userunit = models.CharField(max_length=20,default="Undefined",blank=False,help_text=ugettext_lazy("units of measure"))
    
    def natural_key(self):
        #print "natural key bcode"
        #print self,self.bcode
        return (self.bcode,)

    class Meta:
        ordering = ['bcode']
        verbose_name = 'Variable Bcode' 
        verbose_name_plural = 'Variable Bcode' 
        #unique_together = (('name', 'type'),)

    def describe_var(self):
        if dballepresent:
            varinfo=dballe.varinfo(self.bcode)
            return varinfo.desc.lower()+" "+varinfo.unit
        else:
            return self.bcode

    def describe_uservar(self):
        if dballepresent:
            varinfo=dballe.varinfo(self.bcode)
            return varinfo.desc.lower()+" "+self.userunit
        else:
            return self.bcode
        
    def __str__(self):
        return '%s-%s-%s' % (self.bcode,self.description,self.unit)


    
class TransportRF24NetworkManager(models.Manager):
    def get_by_natural_key(self, board):
        #print "TransportRF24NetworkManager:",board
        return self.get(board=Board.objects.get_by_natural_key(board[0],board[1]))

class TransportRF24Network(models.Model):
    """RF24 Network transport."""

    objects = TransportRF24NetworkManager()

    RF24_NODE_CHOICES = (
        (0,  'RF24 Network node 0'),
        (1,  'RF24 Network node 01'),
        (2,  'RF24 Network node 02'),
        (3,  'RF24 Network node 03'),
        (4,  'RF24 Network node 04'),
        (5,  'RF24 Network node 05'),
    )

    RF24_CHANNEL_CHOICES = (
        (90,  'RF24 Network node channel 90'),
        (91,  'RF24 Network node channel 91'),
        (92,  'RF24 Network node channel 92'),
        (93,  'RF24 Network node channel 93'),
        (94,  'RF24 Network node channel 94'),
        (95,  'RF24 Network node channel 95'),
    )

    RF24_KEYIV_CHOICES= (
        ("0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15",  'preset 1'),
        ("0,1,1,3,4,5,6,7,8,9,10,11,12,13,14,15",  'preset 2'),
        ("0,1,2,1,4,5,6,7,8,9,10,11,12,13,14,15",  'preset 3'),
        ("0,1,2,3,1,5,6,7,8,9,10,11,12,13,14,15",  'preset 4'),
        ("0,1,2,3,4,1,6,7,8,9,10,11,12,13,14,15",  'preset 5'),
        ("0,1,2,3,4,5,1,7,8,9,10,11,12,13,14,15",  'preset 6'),
        ("0,1,2,3,4,5,6,1,8,9,10,11,12,13,14,15",  'preset 7'),
        ("0,1,2,3,4,5,6,7,1,9,10,11,12,13,14,15",  'preset 8'),
        ("0,1,2,3,4,5,6,7,8,1,10,11,12,13,14,15",  'preset 9'),
    )

    active = models.BooleanField(ugettext_lazy("Active"),default=False,null=False,blank=False,help_text=ugettext_lazy("Activate this transport for measurements"))
    node = models.PositiveIntegerField(unique=False,default=0, choices=RF24_NODE_CHOICES,help_text=ugettext_lazy("Node ID for RF24 Network"))
    channel=models.PositiveIntegerField(default=93,null=False,blank=False, choices=RF24_CHANNEL_CHOICES,help_text=ugettext_lazy("Channel number for RF24"))

    #  TODO integer field to be converted to 0X
    key=models.CharField(validators=[validate_comma_separated_integer_list],max_length=47,null=False,blank=True, choices=RF24_KEYIV_CHOICES,help_text=ugettext_lazy("AES key"))
    iv=models.CharField(validators=[validate_comma_separated_integer_list],max_length=47,null=False,blank=True, choices=RF24_KEYIV_CHOICES,help_text=ugettext_lazy("AES cbc iv"))

    board = models.OneToOneField("Board",on_delete=models.CASCADE)

    def natural_key(self):
        #print "natural key TransportRF24Network"
        #print self,self.board.natural_key()
        return (self.board.natural_key(),)

    natural_key.dependencies = ['stations.board']

    class Meta:
        ordering = ['node']
        verbose_name = 'RF24 Network node' 
        verbose_name_plural = 'RF24 Network nodes' 

    def __str__(self):
        return '%s' % (self.node)


class TransportMqttManager(models.Manager):
    def get_by_natural_key(self, board):
        #print "TransportMqttManager: ",board
        return self.get( board=Board.objects.get_by_natural_key(board[0],board[1]))


class TransportMqtt(models.Model):
    """MQTT transport."""

    def genpskkey():
        def randomchars(length):
            s = "1234567890ABCDEF"*30
            return "".join(random.sample(s,length))

        return randomchars(32)
    
    objects = TransportMqttManager()

    active = models.BooleanField(ugettext_lazy("Active"),default=False,help_text=ugettext_lazy("Activate this transport for measurements"))

    mqttsampletime = models.PositiveIntegerField(default=5,null=False,blank=False,help_text=ugettext_lazy("interval in seconds for publish"))
    mqttserver = models.CharField(max_length=50,default="mqttserver",null=False,blank=False,help_text=ugettext_lazy("MQTT server"))
    mqttuser= models.CharField(max_length=9,default="",null=False,blank=True,help_text=ugettext_lazy("MQTT user"))
    mqttpassword= models.CharField(max_length=50,default="",null=True,blank=True,help_text=ugettext_lazy("MQTT password"))
    mqttpskkey= models.CharField(max_length=254,default=genpskkey,null=True,blank=True,help_text=ugettext_lazy("MQTT PSK Key"))

    board = models.OneToOneField("Board",on_delete=models.CASCADE)

    def natural_key(self):
        #print "natural key"
        #print self,self.board.natural_key()
        return (self.board.natural_key(),)

    natural_key.dependencies = ['stations.board']

    class Meta:
        ordering = ['mqttserver']
        verbose_name = 'MQTT transport'
        verbose_name_plural = 'MQTT transport'

    def __str__(self):
        return '%s' % (self.mqttserver)


class TransportCanManager(models.Manager):
    def get_by_natural_key(self, board):
        #print "TransportCanManager: ",board
        return self.get( board=Board.objects.get_by_natural_key(board[0],board[1]))


class TransportCan(models.Model):
    """CAN transport."""

    objects = TransportCanManager()

    active = models.BooleanField(ugettext_lazy("Active"),default=False,help_text=ugettext_lazy("Activate this transport for measurements"))
    cansampletime = models.PositiveIntegerField(default=5,null=False,blank=False,help_text=ugettext_lazy("interval in seconds for publish"))
    node_id = models.PositiveIntegerField(default=100,null=False,blank=False,help_text=ugettext_lazy("Cyphal node_id"))
    subject = models.CharField(max_length=100,default="",null=True,blank=False,help_text=ugettext_lazy("Cyphal subject"))    
    subject_id=models.PositiveIntegerField(default=100,null=False,blank=False,help_text=ugettext_lazy("Cyphal subject-ID (decimal)"))
    board = models.OneToOneField("Board",on_delete=models.CASCADE)

    def natural_key(self):
        #print "natural key"
        #print self,self.board.natural_key()
        return (self.board.natural_key(),)

    natural_key.dependencies = ['stations.board']

    class Meta:
        ordering = ['node_id']
        verbose_name = 'CAN transport'
        verbose_name_plural = 'CAN transport'

    def __str__(self):
        return '%d' % (self.node_id)


class TransportTcpipManager(models.Manager):
    def get_by_natural_key(self, board):
        #print "TransportTcpipManager: ",board
        return self.get(board=Board.objects.get_by_natural_key(board[0],board[1]))

class TransportTcpip(models.Model):
    """TCP/IP transport."""

    objects = TransportTcpipManager()

    TCPIP_NAME_CHOICES = (
        ('master',   'master board 1'),
        ('master2',  'master board 2'),
        ('master3',  'master board 3'),
        ('master4',  'master board 4'),
        ('stima',    'master stima 1'),
        ('stima2',    'master stima 2'),
        ('stima3',    'master stima 3'),
        ('stima4',    'master stima 4'),
    )

    mac={
        'master': (0,0,0,0,0,1) ,
        'master2':(0,0,0,0,0,2),
        'master3':(0,0,0,0,0,3),
        'master4':(0,0,0,0,0,4),
        'master4':(0,0,0,0,0,4),
        'stima': (0xE2,0x21,0xB6,0x44,0xEB,0x29) ,
        'stima2':(0x76,0x63,0x6E,0x28,0xDC,0xE5),
        'stima3':(0x72,0x70,0xD5,0x08,0xC0,0x09),
        'stima4':(0xC2,0x84,0x73,0xC8,0x3C,0xDF),
    }

    active = models.BooleanField(ugettext_lazy("Active"),default=False,help_text=ugettext_lazy("Activate this transport for measurements"))
    name = models.CharField(max_length=50, default="master",blank=False,choices=TCPIP_NAME_CHOICES,help_text=ugettext_lazy("Name DSN solved (for master board only)"))

    ntpserver = models.CharField(max_length=50,default="ntpserver",null=False,blank=False,help_text=ugettext_lazy("Network time server (NTP)"))

    gsmapn = models.CharField(max_length=50,default="ibox.tim.it",null=False,blank=False,help_text=ugettext_lazy("APN for gsm access"))
    pppnumber = models.CharField(max_length=30,default="*99#",null=True,blank=True,help_text=ugettext_lazy("number for LTE PPP access"))

    board = models.OneToOneField("Board",on_delete=models.CASCADE)

    def natural_key(self):
        #print "natural key"
        #print self,self.board.natural_key()
        return (self.board.natural_key(),)

    natural_key.dependencies = ['stations.board']

    class Meta:
        ordering = ['name']
        verbose_name = 'tcp/ip DNS resoved name' 
        verbose_name_plural = 'tcp/ip DNS resoved names' 

    def __str__(self):
        return '%s' % (self.name)

class TransportSerialManager(models.Manager):
    def get_by_natural_key(self, board):
        #print "TransportSerialManager:", board
        return self.get(board=Board.objects.get_by_natural_key(board[0],board[1]))

class TransportSerial(models.Model):
    """Serial transport."""

    objects = TransportSerialManager()

    SERIAL_BAUDRATE_CHOICES = (
        (9600,   '9600'),
        (19200,  '19200'),
        (38400,  '38400'),
        (115200, '115200'),
    )

    SERIAL_DEVICE_CHOICES = (
        ('COM1', 'windows COM1'),
        ('COM2', 'windows COM2'),
        ('COM3', 'windows COM3'),
        ('COM4', 'windows COM4'),
        ('COM5', 'windows COM5'),
        ('COM6', 'windows COM6'),
        ('COM7', 'windows COM7'),
        ('COM8', 'windows COM8'),
        ('COM9', 'windows COM9'),
        ('COM10', 'windows COM10'),
        ('COM11', 'windows COM11'),
        ('COM12', 'windows COM12'),
        ('COM13', 'windows COM13'),
        ('COM14', 'windows COM14'),
        ('COM15', 'windows COM15'),
        ('COM16', 'windows COM16'),
        ('COM17', 'windows COM17'),
        ('COM18', 'windows COM18'),
        ('COM19', 'windows COM19'),
        ('/dev/ttyUSB0', 'Linux ttyUSB0'),
        ('/dev/ttyUSB1', 'Linux ttyUSB1'),
        ('/dev/ttyUSB2', 'Linux ttyUSB2'),
        ('/dev/ttyUSB3', 'Linux ttyUSB3'),
        ('/dev/ttyUSB4', 'Linux ttyUSB4'),
        ('/dev/ttyACM0', 'Linux ttyACM0'),
        ('/dev/ttyACM1', 'Linux ttyACM1'),
        ('/dev/ttyACM2', 'Linux ttyACM2'),
        ('/dev/ttyACM3', 'Linux ttyACM3'),
        ('/dev/ttyACM4', 'Linux ttyACM4'),
        ('/dev/rfcomm0', 'Linux rfcomm0'),
        ('/dev/rfcomm1', 'Linux rfcomm1'),
        ('/dev/rfcomm2', 'Linux rfcomm2'),
        ('/dev/tty.HC-05-DevB', 'OSX tty.HC-05-DevB'),
        ('/dev/tty.usbserial', 'OSX tty.usbserial'),
    )

    active = models.BooleanField(ugettext_lazy("Active"),default=False,help_text=ugettext_lazy("Activate this transport for measurements"))
    baudrate = models.PositiveIntegerField(default=115200,null=False,blank=False,choices=SERIAL_BAUDRATE_CHOICES,help_text=ugettext_lazy("Baud rate"))
    device = models.CharField(max_length=30,unique=False,default="/dev/ttyUSB0",null=False,blank=False, choices=SERIAL_DEVICE_CHOICES,help_text=ugettext_lazy("Serial device"))

    board = models.OneToOneField("Board",on_delete=models.CASCADE)

    def natural_key(self):
        #print "natural key"
        #print self,self.board.natural_key()
        return (self.board.natural_key(),)

    natural_key.dependencies = ['stations.board']

    class Meta:
        ordering = ['board']
        verbose_name = 'serial transport'
        verbose_name_plural = 'serial transports'

    def __str__(self):
        return '%s' % (self.device)


class TransportBluetoothManager(models.Manager):
    def get_by_natural_key(self, board):
        #print "TransportBluetoothManager: ",board
        return self.get(board=Board.objects.get_by_natural_key(board[0],board[1]))

class TransportBluetooth(models.Model):
    """Bluetooth transport."""

    objects = TransportBluetoothManager()

    active = models.BooleanField(ugettext_lazy("Active"),default=False,help_text=ugettext_lazy("Activate this transport for measurements"))
    name = models.CharField(max_length=80,help_text=ugettext_lazy("bluetooth name"))

    board = models.OneToOneField("Board",on_delete=models.CASCADE)

    def natural_key(self):
        #print "natural key"
        #print self,self.board.natural_key()
        return (self.board.natural_key(),)

    natural_key.dependencies = ['stations.board']

    class Meta:
        ordering = ['name']
        verbose_name = 'bluetooth transport'
        verbose_name_plural = 'bluetooth transport'

    def __str__(self):
        return '%s' % (self.name)


class TransportAmqpManager(models.Manager):
    def get_by_natural_key(self, board):
        #print "TransportAmqpManager: ", board
        return self.get(board=Board.objects.get_by_natural_key(board[0],board[1]))

class TransportAmqp(models.Model):
    """amqp transport."""

    objects = TransportAmqpManager()

    active = models.BooleanField(ugettext_lazy("Active"),default=False,help_text=ugettext_lazy("Activate this transport for measurements"))

    amqpserver = models.CharField(max_length=50,default="rmap.cc",null=False,blank=False,help_text=ugettext_lazy("AMQP server"))
    exchange = models.CharField(max_length=50,default="rmap",null=False,blank=False,help_text=ugettext_lazy("AMQP remote exchange name"))
    queue = models.CharField(max_length=50,default="rmap",null=False,blank=False,help_text=ugettext_lazy("AMQP local queue name"))
    amqpuser= models.CharField(max_length=9,default="",null=False,blank=True,help_text=ugettext_lazy("AMQP user"))
    amqppassword= models.CharField(max_length=50,default="",null=False,blank=True,help_text=ugettext_lazy("AMQP password"))

    board = models.OneToOneField("Board",on_delete=models.CASCADE)

    def natural_key(self):
        #print "natural key"
        #print self,self.board.natural_key()
        return (self.board.natural_key(),)

    natural_key.dependencies = ['stations.board']

    def __str__(self):
        return '%s' % (self.amqpserver)


class BoardManager(models.Manager):
    def get_by_natural_key(self, slug,stationmetadata):
        #print "BoardManager: ",slug,stationmetadata
        return self.get(slug=slug,stationmetadata=StationMetadata.objects.get_by_natural_key(stationmetadata[0],stationmetadata[1]))

class Board(models.Model):
    """Board model."""

    objects = BoardManager()

    BOARD_CATEGORY_CHOICES = (
        ('base','Raspberry base'),
        ('master', 'Stima V3 master eth transport'),
        ('slave', 'Stima V3 slave'),
        ('satellite','Stima V3 slave radio transport'),
        ('gsm','Stima V3 master GSM/GPRS transport'),
        ('bluetooth','Stima V3 slave Bluetooth transport'),
        ('masterv4', 'Stima V4 master eth transport'),
        ('gsmv4', 'Stima V4 master LTS transport'),
        ('slavev4', 'Stima V4 slave CAN transport'),        
    )

    STIMAV4_MODULE_TYPE_CHOICES = (
        ( 0,"Module type stima not defined"),
        (10,"Module send report over ethernet"),
        (11,"Module send sample over gsm/gprs"),
        (20,"Module acquire rain tips"),
        (21,"Module acquire temperature and humidity"),
        (22,"Module acquire temperature, humidity and rain"),
        (23,"Module acquire air particle"),
        (24,"Module acquire leaf wetness"),
        (25,"Module acquire wind sensor"),
        (26,"Module acquire radiation sensor"),
        (27,"Module acquire gas (NO2, CO2)"),
        (28,"Module acquire power regulator mppt")
    )

    name = models.CharField(max_length=255,help_text=ugettext_lazy("station name"))
    active = models.BooleanField(ugettext_lazy("Active"),default=False,help_text=ugettext_lazy("Activate the board for measurements"))
    slug = models.SlugField(unique=False, help_text=ugettext_lazy('Auto-generated from name.'))
    category = models.CharField(max_length=50, null= True, blank=True,choices=BOARD_CATEGORY_CHOICES,help_text=ugettext_lazy("General standard category"))
    type = models.PositiveIntegerField(default=0,null=False, blank=False,choices=STIMAV4_MODULE_TYPE_CHOICES,help_text=ugettext_lazy("Stima V4 standard type"))
    sn = models.PositiveIntegerField(default=None,null=True, blank=True,help_text=ugettext_lazy("Serial number"))
    stationmetadata = models.ForeignKey('StationMetadata',on_delete=models.CASCADE)
    
#    def changeform_link(self):
#        if self.id:
#            # Replace "myapp" with the name of the app containing
#            # your Certificate model:
#            changeform_url = urlresolvers.reverse(
#                'admin:station_change', args=(self.id,)
#            )
#            return u'<a href="%s" target="_blank">Details</a>' % changeform_url
#        return u''
#    changeform_link.allow_tags = True
#    changeform_link.short_description = ''   # omit column header

    def natural_key(self):
        #print "natural key"
        #print self,self.slug,self.stationmetadata.natural_key()
        return (self.slug,self.stationmetadata.natural_key())

    natural_key.dependencies = ['stations.stationmetadata']

    class Meta:
        ordering = ['slug']
        verbose_name = 'hardware board'
        verbose_name_plural = 'hardware boards'
        unique_together = (('slug', 'stationmetadata'),)

    def __str__(self):
        return '%s' % (self.slug)



class StationMaintStatus(models.Model):
    """Station status based on maint messages over MQTT."""

    station = models.OneToOneField("StationMetadata",on_delete=models.CASCADE)
    
    laststatus = models.CharField(max_length=128, blank=True,default="",help_text=ugettext_lazy("Last status"))
    lastupdate = models.DateTimeField(null=True,blank=True,help_text=ugettext_lazy("Last status update date"))
    firmwaremajor = models.PositiveIntegerField(default=None,null=True,blank=True,help_text=ugettext_lazy("firmware major version"))
    firmwareminor = models.PositiveIntegerField(default=None,null=True,blank=True,help_text=ugettext_lazy("firmware minor version"))


class BoardMaintStatus(models.Model):
    """Board status based on maint messages over MQTT."""

    board = models.OneToOneField("Board",on_delete=models.CASCADE)

    lastupdate = models.DateTimeField(null=True,blank=True,help_text=ugettext_lazy("Last status update date"))
    
    statusb1  = models.BooleanField(null=True,help_text=ugettext_lazy("Last status bit 1"))
    statusb2  = models.BooleanField(null=True,help_text=ugettext_lazy("Last status bit 2"))
    statusb3  = models.BooleanField(null=True,help_text=ugettext_lazy("Last status bit 3"))
    statusb4  = models.BooleanField(null=True,help_text=ugettext_lazy("Last status bit 4"))
    statusb5  = models.BooleanField(null=True,help_text=ugettext_lazy("Last status bit 5"))
    statusb6  = models.BooleanField(null=True,help_text=ugettext_lazy("Last status bit 6"))
    statusb7  = models.BooleanField(null=True,help_text=ugettext_lazy("Last status bit 7"))
    statusb8  = models.BooleanField(null=True,help_text=ugettext_lazy("Last status bit 8"))
    statusb9  = models.BooleanField(null=True,help_text=ugettext_lazy("Last status bit 9"))
    statusb10 = models.BooleanField(null=True,help_text=ugettext_lazy("Last status bit 10"))
    statusb11 = models.BooleanField(null=True,help_text=ugettext_lazy("Last status bit 11"))
    statusb12 = models.BooleanField(null=True,help_text=ugettext_lazy("Last status bit 12"))
    statusb13 = models.BooleanField(null=True,help_text=ugettext_lazy("Last status bit 13"))
    statusb14 = models.BooleanField(null=True,help_text=ugettext_lazy("Last status bit 14"))
    statusb15 = models.BooleanField(null=True,help_text=ugettext_lazy("Last status bit 15"))
    statusb16 = models.BooleanField(null=True,help_text=ugettext_lazy("Last status bit 16"))

    statusv1 = models.PositiveIntegerField(default=None,null=True,blank=True,help_text=ugettext_lazy("Last status value 1 (%)"))
    statusv2 = models.PositiveIntegerField(default=None,null=True,blank=True,help_text=ugettext_lazy("Last status value 2 (%)"))
    statusv3 = models.PositiveIntegerField(default=None,null=True,blank=True,help_text=ugettext_lazy("Last status value 3 (%)"))
    statusv4 = models.PositiveIntegerField(default=None,null=True,blank=True,help_text=ugettext_lazy("Last status value 4 (%)"))
    statusv5 = models.PositiveIntegerField(default=None,null=True,blank=True,help_text=ugettext_lazy("Last status value 5 (%)"))
    statusv6 = models.PositiveIntegerField(default=None,null=True,blank=True,help_text=ugettext_lazy("Last status value 6 (%)"))
    statusv7 = models.PositiveIntegerField(default=None,null=True,blank=True,help_text=ugettext_lazy("Last status value 7 (%)"))
    statusv8 = models.PositiveIntegerField(default=None,null=True,blank=True,help_text=ugettext_lazy("Last status value 8 (%)"))
    
    
class BoardFirmwareMetadata(models.Model):
    """Board metadata for firmware management."""

    board = models.OneToOneField("Board",on_delete=models.CASCADE)
    
    mac = models.CharField(max_length=128, blank=True,default="",help_text=ugettext_lazy("MAC address"))
    swversion = models.CharField(max_length=255, blank=True,default="",help_text=ugettext_lazy("Software version"))
    swlastupdate = models.DateTimeField(null=True,blank=True,help_text=ugettext_lazy("Software last update date"))

    
class StationConstantDataManager(models.Manager):
    def get_by_natural_key(self, btable,stationmetadata):
        #print "StationConstantDataManager: ", stationmetadata
        return self.get( btable=btable, stationmetadata=StationMetadata.objects.get_by_natural_key(stationmetadata[0],stationmetadata[1]))

class StationConstantData(models.Model):
    """Station constant metadata."""

    objects = StationConstantDataManager()

    BTABLE_CHOICES = (
        ("B01019","LONG STATION OR SITE NAME                                        (CCITTIA5)"),
        ("B02001","TYPE OF STATION                                                  (CODE TABLE 2001)"),
        ("B02002","TYPE OF INSTRUMENTATION FOR WIND MEASUREMENT                     (FLAG TABLE 2002)"),
        ("B02003","TYPE OF MEASURING EQUIPMENT USED                                 (CODE TABLE 2003)"),
        ("B02004","TYPE OF INSTRUMENTATION FOR EVAPORATION MEASUREMENT OR TYPE OF C (CODE TABLE 2004)"),
        ("B02005","PRECISION OF TEMPERATURE OBSERVATION                             (K*100)"),
        ("B02038","METHOD OF WATER TEMPERATURE AND/OR SALINITY MEASUREMENT          (CODE TABLE 2038)"),
        ("B02039","METHOD OF WET-BULB TEMPERATURE MEASUREMENT                       (CODE TABLE 2039)"),
        ("B07030","HEIGHT OF STATION GROUND ABOVE MEAN SEA LEVEL (SEE NOTE 3)       (m*10)"),
        ("B07031","HEIGHT OF BAROMETER ABOVE MEAN SEA LEVEL (SEE NOTE 4)            (m*10)"),
    )

    active = models.BooleanField(ugettext_lazy("Active"),default=True,help_text=ugettext_lazy("Activate this metadata"))
    btable = models.CharField(max_length=6,unique=False,blank=False,choices=BTABLE_CHOICES,help_text=ugettext_lazy("A code to define the metadata. See rmap RFC"))
    value=models.CharField(max_length=32,null=False,blank=False,help_text=ugettext_lazy("value for associated B table"))
    stationmetadata = models.ForeignKey('StationMetadata',on_delete=models.CASCADE)

    def natural_key(self):
        #print "natural key"
        #print self,self.btable, self.stationmetadata.natural_key()
        return (self.btable, self.stationmetadata.natural_key())

    natural_key.dependencies = ['stations.stationmetadata']

    class Meta:
        ordering = ['btable']
        verbose_name = 'Station constant metadata'
        verbose_name_plural = 'Station constant metadata'
        unique_together = (('btable', 'stationmetadata'),)

    def __str__(self):
        return '%s' % (self.btable)

class StationMetadataManager(models.Manager):
    def get_by_natural_key(self, slug, user):
        #print "StationMetadataManager: ", slug,user
        return self.get(slug=slug, user=User.objects.get_by_natural_key(user[0]))

class StationMetadata(models.Model):
    """Station Metadata."""

    objects = StationMetadataManager()

    STATION_CATEGORY_CHOICES = (
        ('good','Beautifull & Good'),
        ('bad', 'Bad & Wrong'),
        ('test','Test & Bugs'),
        ('unknown','Unknown & Missing'),
    )

    name = models.CharField(max_length=255,default="My station",help_text=ugettext_lazy("station name"))
    active = models.BooleanField(ugettext_lazy("Active"),default=True,help_text=ugettext_lazy("Activate the station for measurements"))
    slug = models.SlugField(unique=False, help_text=ugettext_lazy('Auto-generated from name.'))

    user = models.ForeignKey(User,on_delete=models.CASCADE)
    #ident = models.ForeignKey(User, limit_choices_to={'is_staff': True})

    ident = models.CharField(max_length=9,null=False,blank=True,default="", help_text=ugettext_lazy("identifier for mobile station (should be equal to your username)"))

    lat = models.FloatField(ugettext_lazy("Latitude"),default=None,null=True,blank=True, help_text=ugettext_lazy('Precise Latitude of the fixed station'))
    lon = models.FloatField(ugettext_lazy("Longitude"),default=None,null=True,blank=True, help_text=ugettext_lazy('Precise Longitude of the fixed station'))

    network = models.CharField(max_length=50,default="fixed",unique=False,null=False,blank=False, help_text=ugettext_lazy("station network ( mobile for station with mobile coordinate)"))

    mqttrootpath = models.CharField(max_length=100,default="sample",null=False,blank=False,help_text=ugettext_lazy("root mqtt path for publish"))
    mqttmaintpath = models.CharField(max_length=100,default="maint",null=False,blank=False,help_text=ugettext_lazy("maint mqtt path for publish"))
    category = models.CharField(max_length=50, choices=STATION_CATEGORY_CHOICES,help_text=ugettext_lazy("Category of the station"))

    def lon_lat(self):
        if self.lon is None:
            return "None_None"
        return "%d_%d" % (nint(self.lon*100000),nint(self.lat*100000))

    def lonlat(self):
        if self.lon is None:
            return "NoneNone"
        return "%d,%d" % (nint(self.lon*100000),nint(self.lat*100000))


    def status_bits(self):

        status = False
        for board in self.board_set.all():
            status = (status or board.boardmaintstatus.statusb1 or board.boardmaintstatus.statusb2 or board.boardmaintstatus.statusb3 or board.boardmaintstatus.statusb4)
            status = (status or board.boardmaintstatus.statusb5 or board.boardmaintstatus.statusb6 or board.boardmaintstatus.statusb7 or board.boardmaintstatus.statusb8)
            status = (status or board.boardmaintstatus.statusb9 or board.boardmaintstatus.statusb10 or board.boardmaintstatus.statusb11 or board.boardmaintstatus.statusb12)
            status = (status or board.boardmaintstatus.statusb13 or board.boardmaintstatus.statusb14 or board.boardmaintstatus.statusb15 or board.boardmaintstatus.statusb16)
        return status

    def status_vals(self):

        status = False
        for board in self.board_set.all():
            status = (status or board.boardmaintstatus.statusv1 < 100 or board.boardmaintstatus.statusv2 < 100 or board.boardmaintstatus.statusv3)
            status = (status or board.boardmaintstatus.statusv4 < 100 or board.boardmaintstatus.statusv5 < 100 )
        return status
    
    def clean(self):
        # check sensor datalevel with roothpath
        if not self.active: return

        for board in self.board_set.all():
            if not board.active: continue

            for sensor in board.sensor_set.all():
                if not sensor.active: continue

                if self.mqttrootpath != sensor.type.datalevel:
                    raise ValidationError(ugettext_lazy('Station and sensor have different data level; change mqttrootpath or active sensors.'))

        if (self.lat is None and not self.lon is None) or (not self.lat is None and self.lon is None):
            raise ValidationError(ugettext_lazy('Station have only one coordinate defined (lat/lon).'))
            
        if (self.network != "mobile" and self.lat is None) or (self.network == "mobile" and not self.lat is None):
            raise ValidationError(ugettext_lazy('Station network have inconsistent definition of coordinate (lat/lon).'))

        if (self.ident == "" and self.lat is None):
            raise ValidationError(ugettext_lazy('Station without ident need coordinate (lat/lon).'))

        if (not self.ident == "" and not self.lat is None):
            raise ValidationError(ugettext_lazy('Station with ident cannot have coordinate (lat/lon).'))
        
                
    @property
    def geom(self):
        #return PointField({'type': 'Point', 'coordinates': [self.lon, self.lat]})
        if gdalpresent:
            return Point(self.lon, self.lat)
        else:
            return None

    @property
    def popupContent(self):
        return  '\
        <p>\
           user: <a href="/stationsonmap/{}">{}\
           </a>\
        </p>\
        <p>\
           name: {}\
        </p>\
        <p>\
           slug: <a href="/stations/{}/{}">{}\
           </a>\
        </p>'\
        .format(
            self.user,
            self.user,
            self.name,
            self.user,
            self.slug,
            self.slug,
        )


    def natural_key(self):
        #print "StationMetadata natural_key"
        #print self,self.slug, self.ident.natural_key()
        return (self.slug, self.user.natural_key())

    natural_key.dependencies = ['auth.user']

    class Meta:
        ordering = ['slug']
        verbose_name = 'station'
        verbose_name_plural = 'stations'
        unique_together = (('slug', 'user'),('ident', 'lat','lon','network'))

    def __str__(self):
        return '%s/%s' % (self.slug,self.user)


from django.contrib.auth.models import User


class UserProfileManager(models.Manager):
    def get_by_natural_key(self, user):
        #print "UserProfileManager: ",user
        return self.get(user=User.objects.get_by_natural_key(user[0]))

class UserProfile(models.Model):

    objects = UserProfileManager()

    # This field is required.
    user = models.OneToOneField(User,on_delete=models.CASCADE)

    # Other fields here
    accepted_license = models.BooleanField(ugettext_lazy("I accept CC BY 4.0 license"),default=False,null=False,blank=False,help_text=ugettext_lazy("You need to accept CC BY 4.0 license to provide your data"))
    certification = models.CharField(max_length=20, default="ARPA-ER")

    def natural_key(self):
        #print "UserProfile natural_key"
        #print self,self.user.natural_key()
        return (self.user.natural_key(),)

    natural_key.dependencies = ['auth.user']

from django.db.models.signals import post_save

def create_user_profile(sender, instance, created, **kwargs):
    if created:
        UserProfile.objects.create(user=instance)

post_save.connect(create_user_profile, sender=User)


# use this to set user information from registration process
# use signal from registration app
# the registration form require accept license so we set it to True
# and certification to the "auto-registered" default

def registration_user_activated(sender, user, request, **kwargs):
    # if (request.GET.get("accepted_license",False)):     # the request come fron the activation link received by email so no information in it
    myuser=UserProfile.objects.get(user=user)
    myuser.accepted_license=True
    myuser.certification="auto-registered"
    myuser.save()
                        
user_activated.connect(registration_user_activated)



PHOTO_CATEGORY_CHOICES = (
    ('station','Station description'),
    ('others', 'Others'),
)

class StationImage(models.Model):

    active = models.BooleanField(ugettext_lazy("Active"),default=True,null=False,blank=False,help_text=ugettext_lazy("Activate this station image"))
    comment = models.TextField()
    stationmetadata = models.ForeignKey('StationMetadata',on_delete=models.CASCADE)
    date=models.DateTimeField(auto_now=True, auto_now_add=False)
    category = models.CharField(max_length=50, blank=False,choices=PHOTO_CATEGORY_CHOICES)


    image = DeletingImageField(upload_to='stationimage',processors=[Transpose(),ResizeToFit(1024, 1024)],
                                          format='jpeg',
                                          options={'quality': 70})

    image_thumbnail = ImageSpecField(
        source='image',
        processors = [Transpose(),SmartResize(128, 128)],
        format = 'JPEG',
        options = {'quality': 60}
    )

