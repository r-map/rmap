
/*
 * Test and example code for bfix(bfi()/bfx()) bit field insert/extract project and libraries
 */

#include "bfix.h"
#include <ArduinoLog.h>


void setup()
{

  Serial.begin(9600);
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);
  

  long bitfield1, bitfield2, bitfield3, bitfield4, bitfield5;
  
  int BYTES_PER_LONG = sizeof(unsigned long);

    /* 16 bytes unsigned char array plus 7 bytes pad(3 needed for 32 bit systems and 7 needed for 64 bit systems) */
  unsigned char c[16+7];


  Log.notice( "\nTest and example code for bfix(bfi()/bfx()) bit field insert/extract project and libraries\n\n");
    Log.notice( "BYTES_PER_LONG = %d\n\n\n", BYTES_PER_LONG);

    for ( int i = 0 ; i < 16+7 ; i++ )
    {
        c[i]  = 0x00;
    }

    bfi(c, 1, 1, 1, 0);
    bfi(c, 5, 2, 3, 0);
    bfi(c, 17, 4, 15, 0);
    bfi(c, 97, 32, 0xffffffff, 0);
    bfi(c, 49, 8, 0xee, 0);
    bfi(c, 8, 8, 0xff, 0);

    Log.notice(  "Tests on 32 and 64 bit systems.\n\n");
    Log.notice(  "c(should be) = 8d fe f0 00 00 00 ee 00 00 00 00 00 ff ff ff ff\n");

    Log.notice(  "           c = %X %X %X %X %X %X %X %X ",
        c[0], c[1], c[2], c[3], c[4], c[5], c[6], c[7]);
    Log.notice(  "%X %X %X %X %X %X %X %X\n",
        c[8], c[9], c[10], c[11], c[12], c[13], c[14], c[15]);

    bitfield1 = bfx(c, 1, 1, 0);
    bitfield2 = bfx(c, 5, 2, 0);
    bitfield3 = bfx(c, 17, 4, 0);
    bitfield4 = bfx(c, 113, 16, 0);
    bitfield5 = bfx(c, 49, 8, 0);

    Log.notice( 
        "bitfield1(should be 1) = %ld\n"
        "bitfield2(should be 3) = %ld\n"
        "bitfield3(should be 15) = %ld\n"
        "bitfield4(should be 65535) = %ld\n"
        "bitfield5(should be 238) = %ld\n",
        bitfield1, bitfield2, bitfield3, bitfield4, bitfield5);

    if ( BYTES_PER_LONG == 8 )
    {
        unsigned long ff, ee;

        Log.notice(  "\n\nTests on 64 bit systems.\n\n");
        for ( int i = 0 ; i < 16+7 ; i++ )
        {
            c[i]  = 0x00;
        }

        for ( int i = 0 ; i < BYTES_PER_LONG ; i++ )
        {
            ((unsigned char *)&ff)[i] = 0xff;
            ((unsigned char *)&ee)[i] = 0xee;
        }

        bfi(c, 1, 64, ff, 0);
        bfi(c, 65, 64, ee, 0);

        Log.notice(  "c(should be) = ff ff ff ff ff ff ff ff ee ee ee ee ee ee ee ee\n");
        Log.notice(  "           c = %02x %02x %02x %02x %02x %02x %02x %02x ",
            c[0], c[1], c[2], c[3], c[4], c[5], c[6], c[7]);
        Log.notice(  "%02x %02x %02x %02x %02x %02x %02x %02x\n",
            c[8], c[9], c[10], c[11], c[12], c[13], c[14], c[15]);

        bitfield1 = bfx(c, 1, 64, 0);
        bitfield2 = bfx(c, 65, 64, 0);
        bitfield3 = bfx(c, 33, 64, 0);

        Log.notice( 
            "bitfield1(should be ffffffffffffffff) = %016lx\n"
            "bitfield2(should be eeeeeeeeeeeeeeee) = %016lx\n"
            "bitfield3(should be ffffffffeeeeeeee) = %016lx\n",
            bitfield1, bitfield2, bitfield3);
    }

    Log.notice(  "\n\nError tests.\n\n");

    long ret;

    ret = bfi(c, 1, 1, 1, 0);
    Log.notice(  "bfi(): Set bit_offset = 1(return should be 0) = %d\n", ret);

    ret = bfi(c, 0, 1, 1, 0);
    Log.notice(  "bfi(): Set bit_offset = 0(return should be -1) = %d\n", ret);

    ret = bfi(c, 1, 0, 1, 0);
    Log.notice(  "bfi(): Set bit_len = 0,(return should be -2) = %d\n", ret);

    ret = bfi(c, 1, 65, 1, 0);
    Log.notice(  "bfi(): Set bit_len = 65,(return should be -3) = %d\n", ret);

    ret = bfi(c, 1, 1, 1, 3);
    Log.notice(  "bfi(): Set endian = 3,(return should be -4) = %d\n", ret);

    Log.notice(  "\n");

    ret = bfx(c, 1, 1, 0);
    Log.notice(  "bfx(): Set bit_offset = 1(return should be 1) = %d\n", ret);

    ret = bfx(c, 0, 1, 0);
    Log.notice(  "bfx(): Set bit_offset = 0(return should be -1) = %d\n", ret);

    ret = bfx(c, 1, 0, 0);
    Log.notice(  "bfx(): Set bit_len = 0,(return should be -2) = %d\n", ret);

    ret = bfx(c, 1, 65, 0);
    Log.notice(  "bfx(): Set bit_len = 65,(return should be -3) = %d\n", ret);

    ret = bfx(c, 1, 1, 3);
    Log.notice(  "bfx(): Set endian = 3,(return should be -4) = %d\n", ret);

}

void loop(){};
