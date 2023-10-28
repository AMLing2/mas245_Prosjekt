// Written by K. M. Knausgård 2023-10-21

#include <Arduino.h>
#include <FlexCAN_T4.h>
#include <SPI.h>
#include <Wire.h>
#include <string.h>

int meldingerSendt = 0;

namespace {
  CAN_message_t msg;

  FlexCAN_T4<CAN0, RX_SIZE_256, TX_SIZE_16> can0;
  //FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;
}



void canSniff(const CAN_message_t &msg) {
  Serial.print("MB "); Serial.print(msg.mb);
  Serial.print("  OVERRUN: "); Serial.print(msg.flags.overrun);
  Serial.print("  LEN: "); Serial.print(msg.len);
  Serial.print(" EXT: "); Serial.print(msg.flags.extended);
  Serial.print(" TS: "); Serial.print(msg.timestamp);
  Serial.print(" ID: "); Serial.print(msg.id, HEX);
  Serial.print(" Buffer: ");
  for ( uint8_t i = 0; i < msg.len; i++ ) {
    Serial.print(msg.buf[i], HEX); Serial.print(" ");
  } Serial.println();
}




void sendCan();

void setup() {
  Serial.begin(9600);
  can0.begin();
  can0.setBaudRate(250000);
  //can1.begin();
  //can1.setBaudRate(250000);
  can0.enableFIFO();
  can0.enableFIFOInterrupt();
  can0.onReceive(canSniff);

  


}

void loop() {

while(meldingerSendt < 1000) 
  {
    delay(1000);

    sendCan();
    Serial.println(meldingerSendt);
    meldingerSendt = meldingerSendt + 1;

  }
meldingerSendt = 0;
}



void sendCan()
{
  msg.len = 3;
  msg.id = 0x007;
  msg.buf[0] = 0x26;
  msg.buf[1] = 0x42;
  msg.buf[2] = 0x00;
  can0.write(msg);

  //msg.buf[2] = 0x01;
  //Can1.write(msg);
};






