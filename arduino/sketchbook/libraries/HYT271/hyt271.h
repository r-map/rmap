#ifndef _HYT271_h
#define _HYT271_h

#ifdef __cplusplus
extern "C"
{
#endif
	
	#define I2C_HYT271_DEFAULT_ADDRESS					(0x28)
	#define I2C_HYT271_READ_HT_DATA_LENGTH			(4)
	
	unsigned char HYT271_getHT(unsigned long, float *, float *);
	
#ifdef __cplusplus
}
#endif

#endif