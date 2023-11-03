#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <FlexCAN_T4.h>
#include <math.h>

#define OLED_RESET 5
#define OLED_DC 6
#define OLED_CS 10
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

#define JOY_UP 22
#define JOY_DOWN 23
#define JOY_PRESS 19

#define ballRadius 2

#define padWidth 4.0    // Pads har lokal origo i top venstre hjørne
#define padHeight 20.0

// -------------------- Globale variabler --------------------
  // Ball
float xFartBall = -1.0;
float yFartBall = 0.0;
float xPos = OLED_WIDTH/2.0;          //startPosisjon ball
float yPos = OLED_HEIGHT/2.0; 
int xPosNy = 0;
int yPosNy = 0;

  // Paddles
int padX = OLED_WIDTH - padWidth;  // Start pos høyre pad (lokal). 
float padY = (OLED_HEIGHT - padHeight)/2;
int pad2X = 0;                      // Start pos venstre pad
float pad2Y = (OLED_HEIGHT - padHeight)/2;
int pad2Yoppdatert = pad2Y;         // Starter samme, endres senere
int xPosSlave = (OLED_WIDTH-1) - OLED_HEIGHT;
int yPosSlave = OLED_HEIGHT/2 - 1;
float padFart = 0;             // For JoyStick (pass-by-copy)

float padMid;   // Definert i sjekkBallPosisjonOgSprett()
float diffTheta;
float ballMagnitude = 1.0;  // Lengden av fartvektoren.

  // Misc
int masterState = 0;
int startSpill = 0;
// -------------------- Globale variabler --------------------

Adafruit_SSD1306 display(OLED_DC, OLED_RESET, OLED_CS);
FlexCAN_T4<CAN0, RX_SIZE_256, TX_SIZE_16> can0; //Starter can0
CAN_message_t ballUt;
CAN_message_t paddle1Pos;
CAN_message_t masterEllerSlave; //Buf[0] = 1 betyr start spill, buf[1] = 1 betyr at har master state

//Hvis HOST
void bevegBallHost(float &xPos, float &yPos, float xFartBall, float yFartBall){
  display.fillCircle(xPos, yPos, ballRadius, BLACK);  // Sletter gammel ball
  xPos = xPos + xFartBall;
  yPos = yPos + yFartBall;
  display.fillCircle(xPos, yPos, ballRadius, WHITE);  // Tegner ny ball
}

void bevegBallSlave(float &xPos, float &yPos){ //Speilvender ballposisjon
  display.fillCircle((OLED_WIDTH-1)-xPosSlave, (OLED_HEIGHT-1)-yPosSlave, ballRadius, BLACK);
  display.fillCircle((OLED_WIDTH-1)-xPosNy, (OLED_HEIGHT-1)-yPosNy, ballRadius, WHITE);
  xPosSlave = xPosNy; 
  yPosSlave = yPosNy;
}

void bevegPad1(float &padY){    // Sjekker input fra joystick, Opp ned koordinatsystem
  if(!digitalRead(JOY_UP) && padY != 0){  // Joystick opp aktiv?
    padFart = -1;
  }

  if(!digitalRead(JOY_DOWN) && padY != (OLED_HEIGHT-padHeight)){ // Joystick ned aktiv?
    padFart = 1;
  }

  display.fillRect(padX, padY, padWidth, padHeight, BLACK);         // Fjern gammel pad
  display.fillRect(padX, padY+padFart, padWidth, padHeight, WHITE); // Tegn ny pad
  padY = padY + padFart;    // Oppdater neste posisjon
  padFart = 0;              // Reset input sjekk
}

void bevegPad2(float &pad2Y) {
  display.fillRect(pad2X, (OLED_HEIGHT-padHeight)-pad2Y, padWidth, padHeight, BLACK);          //Sletter gamle pad2
  display.fillRect(pad2X, (OLED_HEIGHT-padHeight)-pad2Yoppdatert, padWidth, padHeight, WHITE); //Tegner ny pad2 fra canbus data
  pad2Y = pad2Yoppdatert;
}

void sprettY(float &yFartBall){
  yFartBall = -1 * yFartBall;
}

void sprettX(float &xFartBall){
  xFartBall = -1 * xFartBall;
}

void sprettHorisontalt(float &xFartBall, float &yFartBall){ // Skalerer vektingen i x og y retning
  xFartBall = - ballMagnitude * cos(diffTheta);
  yFartBall =   ballMagnitude * sin(diffTheta);
}

void canSniff(const CAN_message_t &msg){
  if(msg.id==50){               // Ballposisjon
    xPosNy = msg.buf[0];
    yPosNy = msg.buf[1];
  }
  else if(msg.id==30){           // Gruppenummer + 10 - mottar paddleposisjon (motstanders paddle 1)
    pad2Yoppdatert = msg.buf[0]; // Oversetter fra hexa automatisk
  }
  else if(msg.id==15){
    startSpill = msg.buf[0];
    masterState = msg.buf[1];
  }
}

void sendBallPos() {
  //Host funksjon
  ballUt.id = 50;         //Gruppenummer + 50
  ballUt.len = 2;
  ballUt.buf[0] = xPos; 
  ballUt.buf[1] = yPos;
  can0.write(ballUt);
}

void sendPaddle1pos() {
  paddle1Pos.id = 30;     //Gruppenummer+20
  paddle1Pos.len = 1;
  paddle1Pos.buf[0] = padY;
  can0.write(paddle1Pos);
}

void sjekkBallPosisjonOgSprett(){
//HOST FUNKSJON  
  if ( (yPos > ((OLED_HEIGHT-1)-ballRadius)) || (yPos < (0+ballRadius)) ) // Upper & Lower bounds
  {
    sprettY(yFartBall); //Snur y hastighet
  }
  if ((xPos < (padWidth+ballRadius)) &&           // Sjekker om ball treffer paddle2 (venstre)
      ( (yPos < (OLED_HEIGHT-pad2Y)) && (yPos > ((OLED_HEIGHT-padHeight)-pad2Y)))) // Invertert
  {
    padMid = pad2Y + padHeight/2;
    diffTheta = (yPos - padMid)*(PI/3)/(padHeight/2);    // Differanse [radianer] fra -pi/3 til +pi/3
    sprettHorisontalt(xFartBall, yFartBall);
    xFartBall *= -1;    // Gå til høyre, ikke venstre
  }
  if ((xPos > (padX-ballRadius)) && 
      ((yPos < (padY+padHeight)) && (yPos > padY))) // Sjekker om ball treffer paddle1 (høyre)
  {
    padMid = padY + padHeight/2;                       // Nullpunkt til pad
    diffTheta = (yPos - padMid)*(PI/3)/(padHeight/2);  // Differanse [radianer] fra -pi/3 til +pi/3
    sprettHorisontalt(xFartBall, yFartBall);           // Trig bouncer
  }
  // -------------------------- Feilsøking --------------------------
  //Serial.print("   padMid: "); Serial.print(padMid);
  //Serial.print(" | diffTheta: "); Serial.print(diffTheta);
  Serial.print("   pad2 top: "); Serial.print((OLED_HEIGHT-pad2Y));
  Serial.print(" | pad2 bot: "); Serial.print(((OLED_HEIGHT-padHeight)-pad2Y));
  Serial.print(" | Ball x: "); Serial.print(xPos); 
  Serial.print(" | Ball y: "); Serial.println(yPos); 
  // Serial.print(" | xFartBall: "); Serial.print(xFartBall);
  // Serial.print(" | yFartBall: "); Serial.println(yFartBall);
  // -------------------------- Feilsøking --------------------------
}

void ScoreSjekk(int xPos)
{
  //Sjekker scoren
}


void setup() {
  can0.begin();
  can0.setBaudRate(250000);   
  Serial.begin(9600);                  //Brukes bare til funksjonstesting
  display.begin(SSD1306_SWITCHCAPVCC); //Gir 3.3V til skjerm
  display.clearDisplay();
  display.setTextSize(0);
  display.setTextColor(WHITE);
  //Tegn startskjerm TBA
  display.display(); 
  pinMode(JOY_DOWN, INPUT);           //Setter joystick pins til å være input
  pinMode(JOY_UP, INPUT);
  pinMode(JOY_PRESS, INPUT);

  can0.enableFIFO();
  can0.enableFIFOInterrupt();
  can0.onReceive(canSniff);
}

void loop() {

  if(!digitalRead(19)) { // Hvis joystick presses
    startSpill = 1;
    masterState = 1;
    masterEllerSlave.id = 15;
    masterEllerSlave.len = 2;
    masterEllerSlave.buf[0] = 1; // Starter spill - buf[0] leser til startSpill variabel
    masterEllerSlave.buf[1] = 0; // Setter lokal maskin til master - motsatt maskin leser 0 og kjører slavesløyfe
    can0.write(masterEllerSlave);
  }

  while(startSpill != 0) { 
    bevegPad1(padY);    // Lokal pad bevegelse (høyre pad)
    bevegPad2(pad2Y);   // Henter data fra motstander og beveger (venstre pad)
    sendPaddle1pos();

    if (masterState == 1) {
      //Hvis master
      sendBallPos();
      bevegBallHost(xPos, yPos, xFartBall, yFartBall);
      sjekkBallPosisjonOgSprett();
    }
    else if(masterState == 0){
      //Hvis slave
      bevegBallSlave(xPos, yPos);
    }

    can0.disableFIFOInterrupt();  //Stopper interrupts når skjermen tegnes
    display.display();            //Tegner hele bildet etter alle kalkulasjoner har blitt gjort
    can0.enableFIFOInterrupt();
    delay(60);                    // 60 Hz oppdateringsrate under testing
  }
  //Serial.println(__TIMESTAMP__); 
}

