#include "DS3231.h"
#include <string.h>
#include <unistd.h>
#include<stdio.h>
#include <iostream>
#include<fcntl.h>
#include <fstream>
#include<sys/ioctl.h>
#include<unistd.h>
#include<linux/i2c-dev.h>
#define BUFFER_SIZE 19      //0x00 to 0x12
//#define ADDR_DATE 0x03h


using namespace std;
DS3231::DS3231() {//Constructor
}
int bcdToDec(char a) { return ((a/16)*10 + (a%16)); }
int DecTobcd(char a) { return ((a/10)*16 + (a%10)); }
bool DS3231::openFileandRead() {//to open files and use it in other functions
	if((file=open("/dev/i2c-1", O_RDWR)) < 0){
	perror("failed to open the bus\n");
	return 1;}
	else{
		if(ioctl(file, I2C_SLAVE, 0x68)<0){
		perror("Failed to connect to the sensor\n");
      		 return 1;}
	return true;
	}	
	

}
bool DS3231::readClockFormat(){
	 if(openFileandRead()){
        char buffer[1] = {0x02};// msb address
        write(file, buffer, 1);
        char buf[1];
        read(file, buf, 1);
	char checkBit=(buf[0]>>6);//The 6th bit of 02h is shifted by 6
	cout<<bcdToDec(checkBit)<<endl;//checks if 1 or zero after shifting
	if(bcdToDec(checkBit)==0){//returns true if zero(24 hour format)
	close(file);
	 return true;}
	else{
	close(file);
	 return false;}
	}
	
}
int DS3231::century(){
	if(openFileandRead()){
        char buffer[1] = {0x05};
        write(file, buffer, 1);
        char buf[1];
        read(file, buf, 1);
        char centuryBit=(buf[0]>>7);
	if(bcdToDec(centuryBit)==0) return 20;
	else return 19;
	close(file);
	}
}
void DS3231::setRTCDate(bool century,char day, char date, uint8_t month, char year) {
	if(openFileandRead()){
                char buffer[1] = {0x03};//it is set to the address of the day
                write(file, buffer, 1);
                char buf[4];
                read(file, buf, 4);
		month=DecTobcd(month);
		if(century){//if century bit is given as true, set bit 7 of  0x05 register 1 
                month = month|0x80;//it is done to set year starting with 19
                }//if century is given as false,put all the given values to the appropriate registers directly
                char writeBuffer[BUFFER_SIZE];//write all the user input values as zeros
                writeBuffer[0] = 0x03;
                writeBuffer[1] = DecTobcd(day);          //decimal to bcd conversion for input parameters
                writeBuffer[2] = DecTobcd(date);
                writeBuffer[3] = month;
		writeBuffer[4] = DecTobcd(year);
                write(file, writeBuffer, 5);
                close(file);
                }	

}

void DS3231::getRTCDate() {
	if(openFileandRead()){
	
	char buffer[1] = {0x03};//set to address of the day
	write(file, buffer, 1);
	char buf[4];
	read(file, buf, 4);
	cout<<"And the RTC date is "<< bcdToDec(buf[1])<<"/"<<
         bcdToDec(buf[2])<<"/"<<century()//get the century bit
         << bcdToDec(buf[3])<<"and the day of the week is"<<" "
        << bcdToDec(buf[0])<<endl;
	
	close(file);
	
	}	
}
bool DS3231::setAMPMBit(){
	if(openFileandRead()){
        char buffer[1] = {0x02};
        write(file, buffer, 1);
        char buf[1];
        read(file, buf, 1);
	char removeformatbit= buf[0] & 0xBF;//remove 12/24 hr bit
	char maskam = removeformatbit & 0x20;//get am/pm bit
	char am = maskam>>5;//check if 0(am) or 1 (pm)
	if( bcdToDec(am)==0)return true;//if 0 then it is am 
	else return false;
	}
}
void DS3231::getRTCtime() {
       // if(openFileandRead()){
	if(readClockFormat()==true)// for 24 hour format
	{
		if(openFileandRead()){
			char buffer[1] = {0x00};//seconds address
       			 write(file, buffer, 1);
       			 char buf[BUFFER_SIZE];
       			 read(file, buf, BUFFER_SIZE);
       			 cout<<"And the RTC time is "<< bcdToDec(buf[2])<<"(hours):"<<
        		 bcdToDec(buf[1])<<"(minutes):"<< bcdToDec(buf[0])<<"seconds"<<endl;
			close(file);
		}
	}
		else{//if 12 hour format
			if(openFileandRead()){
			char buffer[1] = {0x00};//set to address of the day
        		write(file, buffer, 1);
        		char buf[3];
       			read(file, buf, 3);
			char hours12format = buf[2]&0x1F;//bit 5 and bit 6 hold am/pm and 12/24 bits so not required for hour value
			cout<<"And the RTC time is "<< bcdToDec(hours12format)<<"(hours):"<<
		        bcdToDec(buf[1])<<"(minutes):"<< bcdToDec(buf[0])<<"(seconds)";
			if(setAMPMBit())cout<<"am"<<endl;
			else cout<<"pm"<<endl;
			close(file);
			}
		}
        
}

void DS3231::setRTCtime(char sec, char min, char hrs) {
	 if(readClockFormat()==true){
		if(openFileandRead()){
			char writeBuffer[BUFFER_SIZE];
			writeBuffer[0] = 0x00;
			writeBuffer[1] = DecTobcd(sec);          //decimal to bcd conversion for input parameters
			writeBuffer[2] = DecTobcd(min);
			writeBuffer[3] = DecTobcd(hrs);
			write(file, writeBuffer, 4);
                              //Called read function to check the new time
			close(file);
        	}	
	}
	//char buf[3];
        //read(file, buf, 3);
	
	else{
		if(openFileandRead()){
        	char buffer[1] = {0x00};
	        write(file, buffer, 1);
	        char buf[3];
	        read(file, buf, 2);
		char hours = buf[2]& 0xE0;//remove the lsb data
		hrs = DecTobcd(hrs);
		hrs = hrs & 0x1F;//remove the last 3 bits that contain 24/12 and am/pm bits(not reqd)
		hrs = hours | hrs;// add the above msb and lsbs
		char writeBuffer[BUFFER_SIZE];
                writeBuffer[0] = 0x00;
                writeBuffer[1] = DecTobcd(sec);          //decimal to bcd conversion for input parameters
                writeBuffer[2] = DecTobcd(min);
                writeBuffer[3] = hrs;
                write(file, writeBuffer, 4);
		close(file);
		}
	}
	
}

void DS3231::togglesetClkFormat(){
        if(readClockFormat()){
                if(openFileandRead()){
                char buffer[BUFFER_SIZE];
                 buffer[0] = {0x02};
                buffer[1]=buffer[1]|0x40;
                 write(file, buffer, 2);
                close(file);
                }
        }
}
void DS3231::toggleresetClkFormat(){
        if(readClockFormat()==false){
                if(openFileandRead()){
                char buffer[BUFFER_SIZE];
                 buffer[0] = {0x02};
                 buffer[1]=buffer[1]&0xBF;
                 write(file, buffer, 2);
                close(file);
                }
        }
}


void DS3231::getRTCtemp() {
        if(openFileandRead()){
        char buffer[1] = {0x11};// msb address
        write(file, buffer, 1);
        char buf[2];
        read(file, buf, 2);
        char lsbBit= buf[1]>>6; //bitwise shift by six to get it to bit 0 and 1
        float tempdecimal = lsbBit;//store the values in float as they are decimal values

	char msbBit = buf[0]>>7;// checks if the sign bit is 0 for positive or one for negative temperaure

	float temperature;
	if(bcdToDec(msbBit)==0){
	cout<<"msb is"<<bcdToDec(msbBit)<<endl;
        float temperature = bcdToDec(buf[0]) + ((buf[1]>>6)*0.25);// since lsb has 0.25 resolution
	cout<<"And the RTC temperature is "<<temperature<<" degree celcius"<<endl;
        }
        else{
         float temperature = buf[0] - (tempdecimal*0.25);
        }

	close(file);
}
}
//////////////////Alarm functions below////////////////////

void DS3231::setRTCalarm1(bool dayordate ,char sec, char min, char hr, char day, char date) {
	if(openFileandRead()){
        char writeBuffer[BUFFER_SIZE];
        writeBuffer[0] = 0x07;
        writeBuffer[1] = DecTobcd(sec);        
	writeBuffer[2] = DecTobcd(min);
        writeBuffer[3] = DecTobcd(hr);
	if(dayordate ){//if true then use day
		writeBuffer[4] =  DecTobcd(day)|(0x40);//this sets the bit 7 of 0x0A to 1 for day
	}
	else{//use date
		writeBuffer[4] = DecTobcd(date)&(0xBF);//this sets the bit 7 of 0x0A to 0 for date
	}
        write(file, writeBuffer, 5);
        close(file);
	}
}

void DS3231::setRTCalarm2(bool dayordate ,char sec, char min, char hr, char day, char date) {
	if(openFileandRead()){
        char writeBuffer[BUFFER_SIZE];
        writeBuffer[0] = 0x0B;
        writeBuffer[1] = DecTobcd(min);        
	writeBuffer[2] = DecTobcd(hr);
        
	if(dayordate ){//if true then use day
		writeBuffer[3] =  DecTobcd(day)|(0x40);//this sets the bit 7 of 0x0A to 1 for day
	}
	else{//use date
		writeBuffer[3] = DecTobcd(date)&(0xBF);//this sets the bit 7 of 0x0A to 0 for date
	}
        write(file, writeBuffer, 4);
        close(file);
	}
}

void DS3231::checkflags(bool alarmflags) {//flags are checked in the register 0x0f
	if(openFileandRead()){
	char writeBuffer[1] = {0x0F};
	write(file,writeBuffer,1);
	char buf[1];
	read(file,buf,1);
	if (alarmflags) //if true check for alarm 1
	{
		char alarmmask = buf[0]&(0x01);
		if (alarmmask ==0x01) cout<<"A1f flag is set to 1"<<endl;
		else cout<<"A1f flag is set to 0"<<endl;
		
	}
	else {
		char alarmmask = buf[0]&(0x02);
		if (alarmmask ==0x02) cout<<"A2f flag is set to 1"<<endl;
		else cout<<"A2f flag is set to 0"<<endl;
	}
	close(file);
	}
}

void DS3231::setControlBits(bool alarmflags) {
	if(openFileandRead()){
	char writeBuffer[1] = {0x0E};
	write(file,writeBuffer,1);
	char buf[1];
	read(file,buf,1);
	if (alarm)//for alarm 1
	{
		buf[0] = buf[0]|(0b00000101); //set bit 0(A1IE)and bit 2(INTCN) to 1
	}
	else 
	{
		buf[0] = buf[0]|(0b00000110); //set bit 1(A2IE)and bit 2(INTCN) to 1
	}
	
	write(file,buf,1);
	close(file);
	}
}

void DS3231::getRTCalarmtime(bool alarm) {
	if(openFileandRead()){
	char writeBuffer[1] = {0x07};
   	write(file, writeBuffer, 1);
   	char buf[BUFFER_SIZE];
   	read(file, buf, BUFFER_SIZE);
	if(alarm) {//for alarm 1
	   	cout<<"And the alarm 1 time set is"<<bcdToDec(buf[2])<<"(hours) : "
		<<bcdToDec(buf[1])<<" (minutes): "<<bcdToDec(buf[0])<<
		"(seconds), And Day/Date is "<<bcdToDec(buf[3])<<endl;
	}
	else {
		cout<<"And the alarm 2 time set is"<<bcdToDec(buf[5])<<"(hours) : "
		<<bcdToDec(buf[4])<<" (minutes): "<<", And Day/Date is "
                <<bcdToDec(buf[6])<<endl;
	}
	close(file);
	}
}
////end alarm///////////

int main(){
        DS3231 ds3231;

//	ds3231.readClockFormat();

//	ds3231.getRTCtemp();
//	ds3231.setCentury();
//	ds3231.getRTCDate();
//      ds3231.setRTCDate(false,5,15,07,22);
//	ds3231.getRTCDate();
//	ds3231.toggleresetClkFormat();
//	ds3231.togglesetClkFormat();
//	ds3231.setRTCtime(50, 59, 12);
//	ds3231.getRTCtime();
	


	ds3231.setRTCalarm1(true ,25, 04 , 12, 3, 17);
	ds3231.getRTCalarmtime(true);
        return 0;
}



DS3231::~DS3231() {//Destructor
}
