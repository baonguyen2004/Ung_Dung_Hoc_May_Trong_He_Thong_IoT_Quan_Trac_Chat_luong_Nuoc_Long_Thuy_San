#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#include <SPI.h>
#include <Wire.h>       //using I2C interface
#include <HardwareSerial.h>


#include <time.h>   //thư viện xử lý, tính toán với giá trị thời gian unix

//định nghia các chân tín hiệu của cổng I2C thứ nhất, giao tiếp với INA219
#define SDA_PIN 21
#define SCL_PIN 22

#define EN_DEBUG	1
#if EN_DEBUG
	#define PRINTS(s)					Serial.print(F(s)); 
	#define PRINTLNS(s)				Serial.println(F(s)); 
	#define PRINT(s,v)				{ Serial.print(F(s)); Serial.print(v); }
	#define PRINTF(s1,v, s2)	{ Serial.print(F(s1)); Serial.print(v); Serial.print(F(s2));}
	#define PRINTLN(s,v)			{ Serial.print(F(s)); Serial.println(v); }
	#define PRINTFLN(s1,v, s2){ Serial.print(F(s1)); Serial.print(v); Serial.println(F(s2)); }
	#define PRINTX(s,v)				{ Serial.print(F(s)); Serial.print(v, HEX); }
	#define PRINTLNX(s,v) 		{ Serial.print(F(s)); Serial.println(v, HEX); }
	#define PRINTFX(s1, v, s2){ Serial.print(F(s1)); Serial.print(v, HEX); Serial.print(F(s2));}
	#define PRINTFLNX(s1, v, s2) { Serial.print(F(s1)); Serial.print(v, HEX); Serial.println(F(s2));}
	#define PRINTB(s,v) 			{ Serial.print(F(s)); Serial.print(v, BIN; }
	#define PRINTLNB(s,v) 		{ Serial.print(F(s)); Serial.println(v, BIN; }
	#define PRINTFB(s1, v, s2){ Serial.print(F(s1)); Serial.print(v, BIN); Serial.print(F(s2));}
	#define PRINTFLNB(s1, v, s2) { Serial.print(F(s1)); Serial.print(v, BIN); Serial.println(F(s2));}
#else
	#define PRINTS(s)
	#define PRINTLNS(s)
	#define PRINT(s,v)
	#define PRINTF(s1,v, s2)
	#define PRINTLN(s,v)
	#define PRINTFLN(s1,v, s2)
	#define PRINTX(s,v)	
	#define PRINTLNX(s,v)
	#define PRINTFX(s1, v, s2)
	#define PRINTFLNX(s1, v, s2) 
	#define PRINTB(s,v) 
	#define PRINTLNB(s,v)
	#define PRINTFB(s1, v, s2)
	#define PRINTFLNB(s1, v, s2) 
#endif 


#define EN_RS485_DEBUG  0
  
//----interface with device components------//
#include "interface.h"

//----sensor data-----------------//
#include "sensor_config.h"
