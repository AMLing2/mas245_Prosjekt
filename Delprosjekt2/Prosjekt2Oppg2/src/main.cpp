#include <Arduino.h>
#include <FlexCAN_T4.h>   // FlexCAN Library
#include <SPI.h>
#include <Wire.h>
#include <string.h>

unsigned int meldingerSendt = 0;  // unsigned int so overflow goes to 0

// ------ By K. M. Knausgård ------
namespace {
  CAN_message_t msg_tx;   // Transmit message

  FlexCAN_T4<CAN0, RX_SIZE_256, TX_SIZE_16> can0;
  //FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;
}
// ------ By K. M. Knausgård ------


// canSniff function by tonton81/FlexCAN_T4 on github
// https://github.com/tonton81/FlexCAN_T4/blob/master/README.md
// ========== Displaying receive and transmit ==========
void canSniff_rx(const CAN_message_t &msg) { // Receive
  Serial.print("- Message received:   ");
  Serial.print(" | Mailbox "); Serial.print(msg.mb);
  Serial.print(" | Overrun: "); Serial.print(msg.flags.overrun);
  Serial.print(" |  Length: "); Serial.print(msg.len);
  Serial.print(" | ID: "); Serial.print(msg.id, HEX);
  Serial.print(" | Buffer: ");
  for ( uint8_t i = 0; i < msg.len; i++ ) {
    Serial.print(msg.buf[i], HEX); Serial.print(" ");
  } Serial.println();

  msg_tx = msg;   // Updates tx = rx
}
void canSniff_tx(const CAN_message_t &msg) { // Transmit
  Serial.print("- Message transmitted:");
  Serial.print(" | Mailbox  "); Serial.print(msg.mb);
  Serial.print(" | Overrun: "); Serial.print(msg.flags.overrun);
  Serial.print(" |  Length: "); Serial.print(msg.len);
  Serial.print(" | ID: "); Serial.print(msg.id, HEX);
  Serial.print(" | Buffer: ");
  for ( uint8_t i = 0; i < msg.len; i++ ) {
    Serial.print(msg.buf[i], HEX); Serial.print(" ");
  } Serial.println();
}
// ========== Displaying receive and transmit ==========

void setup() {
  Serial.begin(9600); //For confirmation of received data
  can0.begin();
  can0.setBaudRate(250000);
  can0.enableFIFO();
  can0.enableFIFOInterrupt();
  can0.onReceive(canSniff_rx);  // Received on Teensy from PCAN
  can0.onTransmit(canSniff_tx); // Transmitted from Teensy to PCAN
}

void loop() {
  delay(2000);        // 2 [seconds]

  can0.write(msg_tx);  // returns message from Teensy to PCAN
  
  meldingerSendt = meldingerSendt + 1;
  Serial.print("\nLoop number: ");
  Serial.println(meldingerSendt);
}