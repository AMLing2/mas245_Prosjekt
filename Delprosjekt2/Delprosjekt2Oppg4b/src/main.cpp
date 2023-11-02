#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <FlexCAN_T4.h>

#define OLED_RESET 5
#define OLED_DC 6
#define OLED_CS 10

#define ballRadius 2

#define padWidth = 4
#define padHeight = 20

#define JOY_UP 22
#define JOY_DOWN 23
#define JOY_PRESS 19


//Globale variabler 
int xFart = 1;
int yFart = 1;
int xPos = 64; //startPosisjon ball
int yPos = 32; 
int padX = 127-3; //startposisjon høyre pads topp venstre hjørne
int padY = 32-10;
int xPosSlave = 127-64;
int yPosSlave = 63-32; 
int padFart = 0;
int xPosNy = 0;
int yPosNy = 0;
int pad2X = 0;
int pad2Y = 32-10;
int pad2Yoppdatert = 22;
int masterState = 0;
int startSpill = 0;

Adafruit_SSD1306 display(OLED_DC, OLED_RESET, OLED_CS);
FlexCAN_T4<CAN0, RX_SIZE_256, TX_SIZE_16> can0; //Starter can0
CAN_message_t ballUt;
CAN_message_t paddle1Pos;
CAN_message_t masterEllerSlave; //Buf[0] = 1 betyr start spill, buf[1] = 1 betyr at har master state

// put function declarations here:

//Hvis HOST

void bevegBallHost(int &xPos, int &yPos, int xFart, int yFart){
  display.fillCircle(xPos, yPos, 2, BLACK);  //Sletter gamle ballen
  xPos = xPos + xFart;
  yPos = yPos + yFart;
  display.fillCircle(xPos, yPos, 2, WHITE); //Tegner ny ball
}

void bevegBallSlave(int &xPos, int &yPos){ //Speilvender ballposisjon
  display.fillCircle(127-xPosSlave, 63-yPosSlave, 2, BLACK);
  display.fillCircle(127-xPosNy, 63-yPosNy, 2, WHITE);
  xPosSlave = xPosNy; 
  yPosSlave = yPosNy;
}

void bevegPad1(int &padY){
  if(!digitalRead(22) && padY != 0){ //Sjekker om joystick opp er aktiv
 padFart = -1;                      //Opp ned koordinatsystem
}


if(!digitalRead(23) && padY != 44){ //Sjekker om joystick ned er aktiv //Fungerer ikke - stopper ikke
 padFart = 1; //Endrer fartsvariabel lokalt 
}

  display.fillRect(padX, padY, 4, 20, BLACK);
  display.fillRect(padX, padY+padFart, 4, 20, WHITE);
  padY = padY + padFart;
  padFart = 0;
}

void bevegPad2(int &pad2Y) {

display.fillRect(pad2X, 44-pad2Y, 4, 20, BLACK); //Sletter gamle pad2
display.fillRect(pad2X, 44-pad2Yoppdatert, 4, 20, WHITE); //Tegner ny pad2 fra canbus data
pad2Y = pad2Yoppdatert;
}

void sprettY(int &yFart){
yFart = -1*yFart;
}

void sprettX(int &xFart){
xFart = -1*xFart;
}

void canSniff(const CAN_message_t &msg){
if(msg.id==50){ //Ballposisjon
xPosNy = msg.buf[0];
yPosNy = msg.buf[1];
}
else if(msg.id==30){ //gruppenummer + 10 - mottar paddleposisjon (motstanders paddle 1)
pad2Yoppdatert = msg.buf[0]; //Oversetter fra hexa automatisk
}
else if(msg.id==15){
  startSpill = msg.buf[0];
  masterState = msg.buf[1];
}

}


void sendBallPos() {
  //Host funksjon
  ballUt.id = 50; //Gruppenummer + 50
  ballUt.len = 2;
  ballUt.buf[0] = xPos; 
  ballUt.buf[1] = yPos;
  can0.write(ballUt);
}

void sendPaddle1pos() {
  paddle1Pos.id = 30; //Gruppenummer+20
  paddle1Pos.len = 1;
  paddle1Pos.buf[0] = padY;
  can0.write(paddle1Pos);
}

void sjekkBallPosisjonOgSprett(){
//HOST FUNKSJON  
if (yPos == 63-ballRadius || yPos == 0+ballRadius)
{
  sprettY(yFart); //Snur y hastighet
}
if (xPos == 4+ballRadius && (yPos < 44-pad2Y+20 && yPos > 44-pad2Yoppdatert)) //Sjekker om ball treffer paddle2 (venstre)
{
  sprettX(xFart);
}

if (xPos == 127-4-ballRadius && (yPos < padY+20 && yPos > padY)) //Sjekker om ball treffer paddle1 (høyre)
{
  sprettX(xFart);
}

}


void ScoreSjekk(int xPos)
{
  //Sjekker scoren


}


void setup() {
  can0.begin();
  can0.setBaudRate(250000);   
  Serial.begin(9600); //Brukes bare til funksjonstesting
  display.begin(SSD1306_SWITCHCAPVCC); //Gir 3.3V til skjerm
  display.clearDisplay();
  display.setTextSize(0);
  display.setTextColor(WHITE);
  //Tegn startskjerm TBA
  //display.fillCircle(xPos, yPos, 256, WHITE); //Hvit bakgrunn
  display.display(); 
  pinMode(JOY_DOWN, INPUT); //Setter joystick pins til å være input
  pinMode(JOY_UP, INPUT);
  pinMode(JOY_PRESS, INPUT);


  can0.enableFIFO();
  can0.enableFIFOInterrupt();
  can0.onReceive(canSniff);
}

void loop() {

if(!digitalRead(19)) { //Hvis joystick presses
startSpill = 1;
masterState = 1;
masterEllerSlave.id = 15;
masterEllerSlave.len = 2;
masterEllerSlave.buf[0] = 1; //Starter spill - buf[0] leser til startSpill variabel
masterEllerSlave.buf[1] = 0; //Setter lokal maskin til master - motsatt maskin leser 0 og kjører slavesløyfe
can0.write(masterEllerSlave);
}


while(startSpill != 0) { 

bevegPad1(padY); //Lokal pad bevegelse (høyre pad)
bevegPad2(pad2Y); //Henter data fra motstander og beveger (venstre pad)
sendPaddle1pos();

if (masterState == 1) {
  //Hvis master
  sendBallPos();
  bevegBallHost(xPos, yPos, xFart, yFart);
  sjekkBallPosisjonOgSprett();
}
else if(masterState == 0){
  //Hvis slave
  bevegBallSlave(xPos, yPos);
}



can0.disableFIFOInterrupt(); //Stopper interrupts når skjermen tegnes
display.display(); //Tegner hele bildet etter alle kalkulasjoner har blitt gjort
can0.enableFIFOInterrupt();
delay(100); //10 Hz oppdateringsrate under testing
}
//Serial.println(__TIMESTAMP__); 


}

