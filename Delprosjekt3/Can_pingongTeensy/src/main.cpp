#include <Arduino.h>
#include <FlexCAN_T4.h>
#include <SPI.h>
#include <Wire.h>
#include <MPU6050.h>
#include "I2Cdev.h" //I2C bus
#include <string.h>
#include <stdlib.h>
#include <string>
#include <Adafruit_BusIO_Register.h> //MPU6050 dependency

//MPU6050 code from https://github.com/ElectronicCats/mpu6050/blob/master/examples/IMU_Zero/IMU_Zero.ino

MPU6050 mpu; //Create IMU object
CAN_message_t msgInn;
CAN_message_t msgUt;
CAN_message_t sensorOut;

//https://github.com/tonton81/FlexCAN_T4/blob/master/README.md
FlexCAN_T4<CAN0, RX_SIZE_256, TX_SIZE_16> can0;

void canSniff(const CAN_message_t &msgInn) { //Do operations and return CAN message
Serial.println("");
Serial.print("Received: ");


char hexToString[7] = {0}; //For printing hex data - size = max message length + 1 to avoid array termination errors
for (int i = 0; i < msgInn.len; i++) {
hexToString[i] = msgInn.buf[i];
Serial.print(msgInn.buf[i], HEX);
Serial.print("");
}
Serial.println("");
Serial.print("Received from Raspberry: ");
Serial.print(hexToString);
Serial.println("");
Serial.println("");


}

void setup() {
  can0.begin();
  Serial.begin(9600);
  can0.setBaudRate(250000);
  can0.enableFIFO();
  can0.enableFIFOInterrupt();
  can0.onReceive(canSniff);
  Wire.begin(); //Start I2C bus
  mpu.initialize(); //After wire.begin(); or crash program.

  //Fra https://github.com/ElectronicCats/mpu6050/tree/master
  Serial.println(mpu.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

}

void loop() {

float num = mpu.getAccelerationZ() * (9.81 / 16384); //Signed 15 bit output, adjusted to display m/s^2 
String floatString = String(num); //Convert float to string

delay(3000);
sensorOut.id = 13;
sensorOut.len = floatString.length();

for (uint i = 0; i < sensorOut.len; i++) { //Load outbound CAN frame with float digits
  sensorOut.buf[i] = floatString[i];
}

can0.write(sensorOut);
Serial.println(""); //
Serial.print("Sent to Raspberry: ");
Serial.print(num);
}
