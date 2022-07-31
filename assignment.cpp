#include "DS3231.cpp"
#include "DS3231.h"
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <iostream>
#include <string>
#include <unistd.h>

#define BUFFER_SIZE 19                           //0x00 to 0x12

using namespace std;

int main(){
        DS3231 date;
        date.setRTCDate(1,15,07,22,true);
        return 1;
}
