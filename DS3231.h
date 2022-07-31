
#ifndef DS3231_H_
#define DS3231_H_
#include <iostream>
#include <fstream>
#include <string.h>
#include <unistd.h>
#include<sys/ioctl.h>

#include<linux/i2c-dev.h>




#define BUFFER_SIZE 19 


using namespace std;

class DS3231 {

public:
	DS3231();
	virtual ~DS3231();
	void getRTCtime();
       	void getRTCDate();
	void setCentury();
	int century();
	void getRTCtemp();
	void togglesetClkFormat();
	void toggleresetClkFormat();
	bool readClockFormat();	
	bool setAMPMBit();
	void setRTCtime(char sec, char min, char hrs);
	void  setRTCDate(bool century, char day, char date,uint8_t month, char year);
	void setRTCalarm1(bool dayordate ,char sec, char min, char hr, char day, char date);
	void setRTCalarm2(bool dayordate ,char sec, char min, char hr, char day, char date);
	void checkflags(bool alarmflags);
	void setControlBits(bool alarm);
	void getRTCalarmtime(bool alarm);
	

private:
	int file;
        int bcdtoDec(char b);
	int Dectobcd(char b);
	bool openFileandRead();
	 char buf[BUFFER_SIZE];		
	
	
	 uint8_t ADDR_DAY = 02;
//	 bool readAllRegisters();
//	 bool writeRegister(uint8_t regAddress, char value);
//	 bool writeRegisters(uint8_t regAddress, char* values, int length);	

};
#endif

