#include <mbed.h>
#include <math.h>
#include <USBSerial.h>
#include "LIS3DSH.h"

DigitalOut led(LED1);
USBSerial serial;

LIS3DSH acc(PA_7, PA_6, PA_5, PE_3);

int main() {
  int16_t X, Y, Z;
  float roll, pitch;

  // checks accelerometer is working
  if(acc.Detect() != 1) {
    serial.printf("LIS3DSH Acceleromoter not detected!\n");
    while(1){ };
  }
  while(1) {
    wait(0.5);  
    static char counter=0;
    acc.ReadData(&X, &Y, &Z);           //read X, Y, Z values
    acc.ReadAngles(&roll, &pitch);      //read roll and pitch angles
    serial.printf("%d,%d,%d\n", X, Y, Z);
    //serial.printf("Roll: %f   Pitch: %f\n", roll, pitch);
  }

}