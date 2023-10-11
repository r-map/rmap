from ctypes import *

class sensor(Structure):
    DRIVER_LENGTH= 5
    TYPE_LENGTH  = 5
    MQTT_SENSOR_TOPIC_LENGTH = 38

    _fields_ = [
         ( 'driver',   c_char * DRIVER_LENGTH)                # sensor's string driver
        ,( 'type', c_char * TYPE_LENGTH)                      # sensor's string type
        ,( 'address', c_ubyte)                                # sensor's address
        ,( 'node', c_ubyte)                                   # sensor's node
        ,( 'mqtt_topic', c_char * MQTT_SENSOR_TOPIC_LENGTH)   # sensor's mqtt topic path
    ]

class constantdata(Structure):

    CONSTANTDATA_BTABLE_LENGTH=7
    CONSTANTDATA_VALUE_LENGTH=33    

    _fields_ = [
         ( 'btable', c_char * CONSTANTDATA_BTABLE_LENGTH)     # table B code for constant station data
        ,( 'value', c_char * CONSTANTDATA_VALUE_LENGTH)       # value of constant station data
     ]
    
class configuration(Structure):
    # struct configuration_t
    # EEPROM saved configuration.

    MQTT_SERVER_LENGTH       =      30
    MQTT_ROOT_TOPIC_LENGTH   =      50
    MQTT_MAINT_TOPIC_LENGTH  =      MQTT_ROOT_TOPIC_LENGTH
    MQTT_RPC_TOPIC_LENGTH    =      MQTT_ROOT_TOPIC_LENGTH
    MQTT_USERNAME_LENGTH     =      30
    MQTT_PASSWORD_LENGTH     =      30
    STATIONSLUG_LENGTH       =      30
    BOARDSLUG_LENGTH         =      30
    NTP_SERVER_LENGTH        =      30
    SENSORS_MAX              =      15
    USE_CONSTANTDATA_COUNT   =      3
    ETHERNET_MAC_LENGTH      =      6
    ETHERNET_IP_LENGTH       =      4
    GSM_APN_LENGTH           =      20
    GSM_USERNAME_LENGTH      =      20
    GSM_PASSWORD_LENGTH      =      20

    #By default, Structure and Union fields are aligned in the same way the C compiler does it.
    #It is possible to override this behavior by specifying a _pack_ class attribute in the subclass definition.
    #This must be set to a positive integer and specifies the maximum alignment for the fields.
    _pack_ = 1  # do not align as AVR
    
    _fields_ = [
        ( 'module_main_version', c_ubyte)                         # module main version
        ,( 'module_configuration_version', c_ubyte)               # module configuration version
        ,( 'module_type', c_ubyte)                                # module type
        ,( 'mqtt_port', c_ushort)                                 # mqtt server port
        ,( 'mqtt_server', c_char *MQTT_SERVER_LENGTH)             # mqtt server
        ,( 'mqtt_root_topic', c_char * MQTT_ROOT_TOPIC_LENGTH)    # mqtt root path
        ,( 'mqtt_maint_topic', c_char * MQTT_MAINT_TOPIC_LENGTH)  # mqtt maint path
        ,( 'mqtt_rpc_topic', c_char * MQTT_RPC_TOPIC_LENGTH)      # mqtt subscribe topic
        ,( 'mqtt_username', c_char * MQTT_USERNAME_LENGTH)        # username to compose mqtt username (username/stationslug/boardslug)
        ,( 'mqtt_password', c_char * MQTT_PASSWORD_LENGTH)        # mqtt password
        ,( 'stationslug', c_char * STATIONSLUG_LENGTH)            # station slug to compose mqtt username (username/stationslug/boardslug)
        ,( 'boardslug', c_char * BOARDSLUG_LENGTH)                # board slug to compose mqtt username (username/stationslug/boardslug)
        ,( 'ntp_server', c_char * NTP_SERVER_LENGTH)              # ntp server
        ,( 'sensors', sensor * SENSORS_MAX)                       # SensorDriver buffer for storing sensors parameter
        ,( 'sensors_count', c_ubyte)                              # configured sensors number
        ,( 'report_seconds', c_ushort)                            # seconds for report values
        ,( 'constantdata', constantdata * USE_CONSTANTDATA_COUNT) # Constantdata buffer for storing constant station data parameter
        ,( 'constantdata_count', c_ubyte)                         # configured constantdata number
        #,( 'is_dhcp_enable', c_bool)                             # dhcp status
        #,( 'ethernet_mac', c_ubyte * ETHERNET_MAC_LENGTH)        # ethernet mac
        #,( 'ip', c_ubyte * ETHERNET_IP_LENGTH)                   # ip address
        #,( 'netmask', c_ubyte * ETHERNET_IP_LENGTH)              # netmask
        #,( 'gateway', c_ubyte * ETHERNET_IP_LENGTH)              # gateway
        #,( 'primary_dns', c_ubyte * ETHERNET_IP_LENGTH)          # primary dns
        ,( 'gsm_apn', c_char * GSM_APN_LENGTH)                    # gsm apn
        ,( 'gsm_username', c_char * GSM_USERNAME_LENGTH)          # gsm username
        ,( 'gsm_password', c_char * GSM_PASSWORD_LENGTH)          # gsm password
    ]

if __name__ == '__main__':
    
    with open('config.cfg', 'rb') as file:
        myconfiguration = configuration()
        bytesread =file.readinto(myconfiguration)

    if(bytesread == sizeof(myconfiguration)):
    
        print (myconfiguration.module_main_version)
        print (myconfiguration.module_configuration_version)
        print (myconfiguration.module_type)        
        print (myconfiguration.mqtt_port)
        print (myconfiguration.mqtt_server)
        print (myconfiguration.mqtt_root_topic)
        print (myconfiguration.mqtt_maint_topic)
        print (myconfiguration.mqtt_rpc_topic)
        print (myconfiguration.mqtt_username)
        print (myconfiguration.mqtt_password)
        print (myconfiguration.stationslug)
        print (myconfiguration.boardslug)
        print (myconfiguration.ntp_server)
        print (myconfiguration.report_seconds)

        print(myconfiguration.sensors_count)
        for sensor in myconfiguration.sensors:
            print(sensor.driver)
            print(sensor.type)
            print(sensor.address)
            print(sensor.node)
            print(sensor.mqtt_topic)

        print (myconfiguration.constantdata_count)
        for constantdata in myconfiguration.constantdata:
            print (constantdata.btable)
            print (constantdata.value)

        print (myconfiguration.gsm_apn)
        print (myconfiguration.gsm_username)
        print (myconfiguration.gsm_password)
