#ifndef _SDS011_H
#define _SDS011_H

#if ARDUINO >= 100
 #include "Arduino.h"
 #include "Print.h"
#else
 #include "WProgram.h"
#endif

#include <Stream.h>

namespace sds011 {
    enum Command {
        CMD_MODE = 2,
        CMD_QUERY_DATA = 4,
        CMD_DEVICE_ID,
        CMD_SLEEP,
        CMD_FIRMWARE,
        CMD_WORKING_PERIOD
    };

    enum Report_mode {
        ACTIVE = 0,
        QUERY = 1
    };

    enum Sleep_work {
        WORK = 0,
	SLEEP = 1
    };
    
    class Sds011
    {
        public:
            Sds011(Stream &out);
            String firmware_version();
            bool set_mode(Report_mode mode);
            bool set_sleep(bool sleep);
            bool query_data(int *pm25, int *pm10);
            bool query_data_auto(int *pm25, int *pm10, int n);
            bool crc_ok();

        private:
            void _send_cmd(enum Command cmd, uint8_t *buf, uint8_t len);
            bool _read_response();
            void _filter_data(int n, int *pm25_table, int *pm10_table, int *pm25, int *pm10);

            Stream &_out;
            uint8_t _buf[19];
    };
}
#endif
