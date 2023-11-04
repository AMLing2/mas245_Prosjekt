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
#define scoreBoxH 11
#define scoreBoxW 35
#define scoreBoxRadius 3

#define periode 10      // Bruker lavere frekvens ved testing. frekvens = periode^-1 [Hz]
#define winScore 10

// -------------------- Globale variabler --------------------
  // Ball
float xFartBall = 0.0;
float yFartBall = 0.0;
float xPos = OLED_WIDTH/2.0;          //startPosisjon ball
float yPos = OLED_HEIGHT/2.0; 
int xPosNy = 0;
int yPosNy = 0;

  // Paddles
int padX = OLED_WIDTH - padWidth;  // Start pos høyre pad (lokal)
float padY = (OLED_HEIGHT - padHeight)/2;
int pad2X = 0;                      // Start pos venstre pad
float pad2Y = (OLED_HEIGHT - padHeight)/2;  // Lokal oppdatering
float pad2Yoppdatert = pad2Y;                 // Fra CANbus ~ starter samme, endres senere
float xPosSlave = (OLED_WIDTH-1) - OLED_HEIGHT;
float yPosSlave = OLED_HEIGHT/2 - 1;
float padFart = 0;             // For JoyStick (pass-by-copy)

float padMid;   // Definert i sjekkBallPosisjonOgSprett()
float diffTheta;
float ballMagnitude = 1.0;  // Lengden av fartvektoren.

  // Misc
bool masterState = false;
int startSpill = 0;       // 0 = start Screen, 1 = game on, 2 = game finished
int16_t X, Y;
uint16_t W, H;
int16_t cursorX = OLED_WIDTH/2;               // Middle
int16_t cursorY = 0;                          // Top
int16_t scoreCursorX = cursorX - scoreBoxW/2;
int hostScore = 9;
int slaveScore = 9;
float t = 0.0;
// -------------------- Globale variabler --------------------

Adafruit_SSD1306 display(OLED_DC, OLED_RESET, OLED_CS);
FlexCAN_T4<CAN0, RX_SIZE_256, TX_SIZE_16> can0; //Starter can0
CAN_message_t ballUt;
CAN_message_t paddle1Pos;
CAN_message_t masterEllerSlave; //Buf[0] = 1 betyr start spill, buf[1] = 1 betyr at har master state
CAN_message_t score;

//Hvis HOST
void bevegBallHost(float &xPos, float &yPos, float xFartBall, float yFartBall){
  //display.fillCircle(xPos, yPos, ballRadius, BLACK);  // Sletter gammel ball
  xPos = xPos + xFartBall;
  yPos = yPos + yFartBall;
  display.fillCircle(xPos, yPos, ballRadius, WHITE);  // Tegner ny ball
}

void bevegBallSlave(float &xPos, float &yPos){ //Speilvender ballposisjon
  //display.fillCircle((OLED_WIDTH-1)-xPosSlave, yPosSlave, ballRadius, BLACK);
  display.fillCircle((OLED_WIDTH-1)-xPosNy, yPosNy, ballRadius, WHITE);
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

  //display.fillRect(padX, padY, padWidth, padHeight, BLACK);         // Fjern gammel pad
  display.fillRect(padX, padY+padFart, padWidth, padHeight, WHITE); // Tegn ny pad
  padY = padY + padFart;    // Oppdater neste posisjon
  padFart = 0;              // Reset input sjekk
}

void bevegPad2(float &pad2Y) {
  //display.fillRect(pad2X, pad2Y, padWidth, padHeight, BLACK);          //Sletter gamle pad2
  display.fillRect(pad2X, pad2Yoppdatert, padWidth, padHeight, WHITE); //Tegner ny pad2 fra canbus data
  pad2Y = pad2Yoppdatert;
}

void sprettY(float &yFartBall){
  yFartBall = -1 * yFartBall;
}

void sprettHorisontalt(float &xFartBall, float &yFartBall){ // Skalerer vektingen i x og y retning
  xFartBall = - ballMagnitude * cos(diffTheta);
  yFartBall =   ballMagnitude * sin(diffTheta);
}

void canSniff(const CAN_message_t &msg){    // KUN for receiving
  if (msg.id == 50){               // Ballposisjon
    xPosNy = msg.buf[0];
    yPosNy = msg.buf[1];
  }
  else if (msg.id == 30){           // Gruppenummer + 10 - mottar paddleposisjon (motstanders paddle 1)
    pad2Yoppdatert = msg.buf[0];      // Oversetter fra hexa automatisk
  }
  else if (msg.id == 15){
    startSpill = msg.buf[0];
    masterState = msg.buf[1];
  }
  else if (msg.id == 10){     // Score
    hostScore = msg.buf[0];
    slaveScore = msg.buf[1];
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

void sendScore(int hostScore, int slaveScore) {
  //Host funksjon
  score.id = 10;
  score.len = 2;
  score.buf[0] = slaveScore;  // Invertert til motstander
  score.buf[1] = hostScore;
  can0.write(score);
}

void sjekkBallPosisjonOgSprett(){
//HOST FUNKSJON  
  if ( (yPos > ((OLED_HEIGHT-1)-ballRadius)) || (yPos < (0+ballRadius)) ) // Upper & Lower bounds
  {
    sprettY(yFartBall); //Snur y hastighet
  }
  if ((xPos < (padWidth+ballRadius)) &&           // Sjekker om ball treffer paddle2 (venstre)
      ( (yPos < (pad2Y+padHeight+ballRadius)) && (yPos > (pad2Y-ballRadius))))
  {
    padMid = pad2Y + padHeight/2;   // Kanskje må endres hvis koordinatsystem er fucked her og
    diffTheta = (yPos - padMid)*(PI/3)/(padHeight/2+ballRadius);    // Differanse [radianer] fra -pi/3 til +pi/3
    sprettHorisontalt(xFartBall, yFartBall);
    xFartBall *= -1;    // Gå til høyre, ikke venstre
  }
  if ((xPos > (padX-ballRadius)) && 
      ((yPos < (padY+padHeight+ballRadius)) && (yPos > (padY-ballRadius)))) // Sjekker om ball treffer paddle1 (høyre)
  {
    padMid = padY + padHeight/2;                       // Nullpunkt til pad
    diffTheta = (yPos - padMid)*(PI/3)/(padHeight/2+ballRadius);  // Differanse [radianer] fra -pi/3 til +pi/3
    sprettHorisontalt(xFartBall, yFartBall);           // Trig bouncer
  }
  // -------------------------- Feilsøking --------------------------
  //Serial.print("   padMid: "); Serial.print(padMid);
  //Serial.print(" | diffTheta: "); Serial.print(diffTheta);
  Serial.print("   pad1 (top): "); Serial.print(padY);
  Serial.print(" | pad2 (top): "); Serial.print(pad2Y);
  Serial.print(" | Ball x: "); Serial.print(xPos); 
  Serial.print(" | Ball y: "); Serial.println(yPos); 
  // Serial.print(" | xFartBall: "); Serial.print(xFartBall);
  // Serial.print(" | yFartBall: "); Serial.println(yFartBall);
  // -------------------------- Feilsøking --------------------------
}

void resetGame(){
  display.clearDisplay();
  xPos = OLED_WIDTH/2.0;          //startPosisjon ball
  yPos = OLED_HEIGHT/2.0; 
  padY = (OLED_HEIGHT - padHeight)/2;
  pad2Y = (OLED_HEIGHT - padHeight)/2;  // Lokal oppdatering
  pad2Yoppdatert = pad2Y;
}

void gameOver(){
  //slaveScore = (xPos < 0) ? slaveScore++ : slaveScore;
  if ((xPos < 0)){
    hostScore++;
    delay(500);
    resetGame();
    xFartBall = 0.5;
    yFartBall = 0.5;
  }
  else if (xPos > OLED_WIDTH){
    slaveScore++;
    delay(500);
    resetGame();
    xFartBall = -0.5;
    yFartBall = 0.5;
  }
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

  // ========================================== START SCREEN ==========================================
  while (startSpill == 0) {
      // Tittel
    display.setTextSize(2);
    display.getTextBounds("PONG!", cursorX, cursorY, &X, &Y, &W, &H);
    display.setCursor((cursorX-W/2), (cursorY+2));
    display.print("PONG!");
      // Svevende Ball
    yFartBall = sin(2*PI*t);
    //xFartBall = cos(t);
    display.fillCircle(xPos, yPos, (ballRadius+1), BLACK);  // Sletter gammel ball
    yPos = yPos + yFartBall;
    //xPos = xPos + xFartBall;
    display.fillCircle(xPos, yPos, (ballRadius+1), WHITE);  // Tegner ny ball

      // tekst
    display.setTextSize(0);
    display.getTextBounds("START: Trykk joystick", cursorX, cursorY, &X, &Y, &W, &H);
    display.setCursor((cursorX-W/2), (OLED_HEIGHT - 10));
    display.print("START: Trykk joystick");

    display.setCursor(1, 1);
    display.print("Gr.5");
    display.getTextBounds("2023", cursorX, cursorY, &X, &Y, &W, &H);
    display.setCursor((OLED_WIDTH-W), 1);
    display.print("2023");

    //display.getTextBounds("2023", cursorX, cursorY, &X, &Y, &W, &H);
    //display.setCursor((OLED_WIDTH-W-1), (OLED_HEIGHT-1));
    //display.print("2023");


    can0.disableFIFOInterrupt();  //Stopper interrupts når skjermen tegnes
    display.display();            //Tegner hele bildet etter alle kalkulasjoner har blitt gjort
    can0.enableFIFOInterrupt();
    delay(periode); 
    t += 0.05;

    if(!digitalRead(JOY_PRESS)) { // Hvis joystick presses
    startSpill = 1;
    masterState = true;
    masterEllerSlave.id = 15;
    masterEllerSlave.len = 2;
    masterEllerSlave.buf[0] = 1; // Starter spill - buf[0] leser til startSpill variabel
    masterEllerSlave.buf[1] = 0; // Setter lokal maskin til master - motsatt maskin leser 0 og kjører slavesløyfe
    can0.write(masterEllerSlave);
    xFartBall = 0.5;
    yFartBall = 0.5;
    //display.display();
  }
  }
  // ========================================== START SCREEN ==========================================

  // ========================================== GAME SCREEN ==========================================
  while(startSpill == 1) {  // Game on
    display.clearDisplay();
    bevegPad1(padY);    // Lokal pad bevegelse (høyre pad)
    bevegPad2(pad2Y);   // Henter data fra motstander og beveger (venstre pad)
    sendPaddle1pos();

    if (masterState) {
      //Hvis master
      sendBallPos();
      bevegBallHost(xPos, yPos, xFartBall, yFartBall);
      sjekkBallPosisjonOgSprett();
      gameOver();
      sendScore(hostScore, slaveScore);
    }
    else if(!masterState){
      //Hvis slave
      bevegBallSlave(xPos, yPos);
    }

    // --------------------------- Score display --------------------------------
    display.drawRoundRect(scoreCursorX, cursorY, scoreBoxW, scoreBoxH, scoreBoxRadius, WHITE);       // Hvit boks
    display.drawRoundRect((scoreCursorX+1), (cursorY+1), scoreBoxW-2, scoreBoxH-2, scoreBoxRadius, BLACK); // Svart boks
    
    display.setCursor((scoreCursorX+3), (cursorY+2));
    display.print(slaveScore);
    display.print(" - ");
    display.print(hostScore);
    //display.setCursor((OLED_WIDTH/2-3), 0); // Middle minus 5 pixels (width of "-")
    //display.print("-");
    // --------------------------- Score display --------------------------------

    can0.disableFIFOInterrupt();  //Stopper interrupts når skjermen tegnes
    display.display();            //Tegner hele bildet etter alle kalkulasjoner har blitt gjort
    can0.enableFIFOInterrupt();
    delay(periode);

    //startSpill = ((slaveScore==winScore)||(hostScore==winScore)) ? 2 : 1;   // Game finished?
    if ((slaveScore==winScore)||(hostScore==winScore)){
      startSpill = 2;
      xPos = OLED_WIDTH/2;
      yPos = OLED_HEIGHT-(ballRadius+8);
    }

  }
  // ========================================== GAME SCREEN ==========================================

  // ========================================== GAME FINISHED ==========================================
  while (startSpill == 2){
    display.clearDisplay();

    if (slaveScore == 10){
      display.setTextSize(1);
      display.getTextBounds("Motstanderen vant!", cursorX, cursorY, &X, &Y, &W, &H);
      display.setCursor((cursorX-W/2), (OLED_HEIGHT/4));
      display.print("Motstanderen vant!");
      display.setTextSize(0);
      display.getTextBounds("Du suger.", cursorX, cursorY, &X, &Y, &W, &H);
      display.setCursor((cursorX-W/2), (OLED_HEIGHT/4 + 10));
      display.print("Du suger.");
      startSpill = 2;
    }
    else if (hostScore == 10){
      display.setTextSize(1);
      display.getTextBounds("Du vant!", cursorX, cursorY, &X, &Y, &W, &H);
      display.setCursor((cursorX-W/2), (OLED_HEIGHT/4));
      display.print("Du vant!");
      display.setTextSize(0);
      display.getTextBounds("Motstanderen suger.", cursorX, cursorY, &X, &Y, &W, &H);
      display.setCursor((cursorX-W/2), (OLED_HEIGHT/4 + 10));
      display.print("Motstanderen suger.");
      startSpill = 2;
    }

    // Svevende Ball
    yFartBall = sin(PI*t);
    xFartBall = cos(0.5*t-PI/4);
    //display.fillCircle(xPos, yPos, (ballRadius+1), BLACK);  // Sletter gammel ball
    yPos = yPos + yFartBall;
    xPos = xPos + xFartBall;
    display.fillCircle(xPos, yPos, (ballRadius+1), WHITE);  // Tegner ny ball

    can0.disableFIFOInterrupt();  //Stopper interrupts når skjermen tegnes
    display.display();            //Tegner hele bildet etter alle kalkulasjoner har blitt gjort
    can0.enableFIFOInterrupt();
    delay(periode); 
    t += 0.1;
  }
  // ========================================== GAME FINISHED ==========================================
  //Serial.println(__TIMESTAMP__); 
}

