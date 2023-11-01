// Written by K. M. Knausgård 2023-10-21

#include <Arduino.h>
#include <FlexCAN_T4.h>
#include <SPI.h>
#include <Wire.h>
#include <string.h>

unsigned int meldingerSendt = 0;  // unsigned int so overflow goes to 0

namespace {
  CAN_message_t msg;

  FlexCAN_T4<CAN0, RX_SIZE_256, TX_SIZE_16> can0;
  //FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;
}

void canMsgInit();

void canSniff(const CAN_message_t &msg) {
  Serial.print("MB "); Serial.print(msg.mb);
  //Serial.print("  OVERRUN: "); Serial.print(msg.flags.overrun);
  Serial.print("|  LENGTH: "); Serial.print(msg.len);
  //Serial.print(" EXT: "); Serial.print(msg.flags.extended);
  //Serial.print(" TS: "); Serial.print(msg.timestamp);
  Serial.print("| ID: "); Serial.print(msg.id, HEX);
  Serial.print("| Buffer: ");
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

      delay(1000);
      //can0.write(msg);
      sendCan();
      meldingerSendt = meldingerSendt + 1;
      Serial.println(meldingerSendt);
}

void sendCan()
{
  can0.write(msg);  // Fra Teensy til PCAN-View
  Serial.println("\n------------------------------------\n");
  Serial.println("\nMessage transmitted from PCAN-View:");
  //msg.buf[2] = 0x01;
  //Can1.write(msg);
};