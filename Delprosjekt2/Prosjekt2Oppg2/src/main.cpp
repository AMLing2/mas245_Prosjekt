// Written by K. M. Knausgård 2023-10-21

#include <Arduino.h>
#include <FlexCAN_T4.h>
#include <SPI.h>
#include <Wire.h>
#include <string.h>

unsigned int meldingerSendt = 0;  // unsigned int so overflow goes to 0

namespace {
  CAN_message_t msg_tx;
  CAN_message_t msg_rx;

  FlexCAN_T4<CAN0, RX_SIZE_256, TX_SIZE_16> can0;
  //FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;
}

void txInit(CAN_message_t &msg_tx) {
  msg_tx.len = 3;
  msg_tx.id = 0x245;
  msg_tx.buf[0] = 0x12;
  msg_tx.buf[1] = 0x34;
  msg_tx.buf[2] = 0x56;
};

void rxOverwrite(CAN_message_t rx, CAN_message_t tx){
  for ( int i = 0; i < rx.len; i++ ) {
    rx.buf[i] = tx.buf[i];
  }
};

// ========== Displaying receive and transmit ==========
void canSniff_rx(const CAN_message_t &msg_rx) { // Receive
  Serial.print("- Message received:   ");
  Serial.print(" | Mailbox "); Serial.print(msg_rx.mb);
  Serial.print(" | Overrun: "); Serial.print(msg_rx.flags.overrun);
  Serial.print(" |  Length: "); Serial.print(msg_rx.len);
  Serial.print(" | ID: "); Serial.print(msg_rx.id, HEX);
  Serial.print(" | Buffer: ");
  for ( uint8_t i = 0; i < msg_rx.len; i++ ) {
    Serial.print(msg_rx.buf[i], HEX); Serial.print(" ");
  } Serial.println();
}
void canSniff_tx(const CAN_message_t &msg_tx) { // Transmit
  Serial.print("- Message transmitted:");
  Serial.print(" | Mailbox  "); Serial.print(msg_tx.mb);
  Serial.print(" | Overrun: "); Serial.print(msg_tx.flags.overrun);
  Serial.print(" |  Length: "); Serial.print(msg_tx.len);
  Serial.print(" | ID: "); Serial.print(msg_tx.id, HEX);
  Serial.print(" | Buffer: ");
  for ( uint8_t i = 0; i < msg_tx.len; i++ ) {
    Serial.print(msg_tx.buf[i], HEX); Serial.print(" ");
  } Serial.println();
}
// ========== Displaying receive and transmit ==========

//void sendCan()
//{
//  can0.write(msg_tx);  // Fra Teensy til CANbus
//  Serial.println("\n------------------------------------");
//  Serial.println("Message transmitted from Teensy.");
  //msg.buf[2] = 0x01;
  //Can1.write(msg);
//};

void setup() {
  Serial.begin(9600);
  can0.begin();
  can0.setBaudRate(250000);
  //can1.begin();
  //can1.setBaudRate(250000);
  can0.enableFIFO();
  can0.enableFIFOInterrupt();

  can0.onReceive(canSniff_rx);
  can0.onTransmit(canSniff_tx);
  can0.events();

  txInit(msg_tx);   // Initialize tx message
}

void loop() {

      delay(2000);
      can0.write(msg_tx); // from Teensy to CAN
      can0.read(msg_rx);  // 
      //sendCan();
      meldingerSendt = meldingerSendt + 1;
      Serial.print("\nRunde nummer: ");
      Serial.println(meldingerSendt);
}