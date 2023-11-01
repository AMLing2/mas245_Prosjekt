
#include <Arduino.h>
#include <FlexCAN_T4.h>
#include <SPI.h>
#include <Wire.h>
#include <string.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_DC 6 //Definerer LCD pins fra kortskjematikk
#define OLED_CS 10 
#define OLED_MOSI 11
#define OLED_RESET 5

Adafruit_SSD1306 display(128,64, &SPI, 6, 5, 10); //Instantierer skjerm

int meldingerSendt = 0;
int canAntallCursorX = 0;
int canAntallCursorY = 0;
int canIdCursorX = 0;
int canIdCursorY = 0;
int meldingerMottatt = 0;
String meldingId;

namespace {
  CAN_message_t msg;

  FlexCAN_T4<CAN0, RX_SIZE_256, TX_SIZE_16> can0;
}

void canSniff(const CAN_message_t &msg);

void setup() {
  Serial.begin(9600);
  can0.begin();
  can0.setBaudRate(250000);


  

  display.begin(SSD1306_SWITCHCAPVCC); //Generate 3.3v
  display.clearDisplay();
 
  display.setTextSize(0);
  display.setTextColor(WHITE);

  display.drawRoundRect(0 ,0, 128, 64, 5, WHITE); //Tegner rektangel
  display.display(); //Sender endringer i RAM til skjerm

  //Print gruppenavn på midten av skjermen, med pause for dramatikk
  delay(1000); //Dramatisk pause som om dette programmet er tungt og krever innlasting
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds("MAS245 - Gruppe 5", 0, 0, &x1, &y1, &w, &h); //Finner lengden på tittel og sender til w og h
  display.setCursor(64-(w/2), 2); //Setter cursor midt på skjermen minus tekstlengde offset
  display.print("MAS245 - Gruppe 5");
  display.display();

  delay(500);

  for(int i = 1; i < 129; i++)
    {
      display.drawLine((i-1),(5*std::sin((i-1)*(3.1415/128))+9), i,(5*std::sin((i-1)*(3.1415/128))+9), WHITE);
      display.display();
      delay(10);
    }



  delay(500);
  display.setCursor(0,15);
  display.println(" CAN-statistikk");
  delay(200);
  display.display();
  display.println(" -----------");
  delay(200);
  display.display();
  display.print(" Antall mottatt: ");
  canAntallCursorX = display.getCursorX(); //Husker cursorkoordinat for å printe tall her senere
  canAntallCursorY = display.getCursorY(); 
  display.println("0");
  display.display();
  delay(200);
  display.print(" Mottok sist ID: ");
  canIdCursorX = display.getCursorX();    //Husker cursorkoordinat for å printe tall her senere
  canIdCursorY = display.getCursorY();
  display.println("0");
  display.display();
  delay(200);
  display.println(" -----------");
  display.display();
  delay(200);

  display.print(" IMU-M");
  display.write(0x86); //Nesten "å"
  display.println("ling Z: TBA");

  display.display();

  
  delay(1000);

  can0.enableFIFO(); //Disse må komme _etter_ tegningen, hvis ikke resetter koordinatene til cursor seg og krøller til alt
  can0.enableFIFOInterrupt();
  can0.onReceive(canSniff); //Kjører canSniff ved mottak av CANbus meldinger
}




void loop() {
//Venter bare på CAN-meldinger
}


void canSniff(const CAN_message_t &msg) { //Kjører når en melding blir mottatt
 
  meldingerMottatt = meldingerMottatt +1;
  meldingId = msg.id;
  display.setCursor(canAntallCursorX, canAntallCursorY); 
  display.fillRect(canAntallCursorX, canAntallCursorY, 20, 7, BLACK); //Sletter hva enn som stod her tidligere
  display.setCursor(canAntallCursorX, canAntallCursorY);
  display.print(meldingerMottatt);


  display.setCursor(canIdCursorX, canIdCursorY);
  display.fillRect(canIdCursorX, canIdCursorY, 20, 7, BLACK);
  display.setCursor(canIdCursorX, canIdCursorY);
  display.print(msg.id, HEX);

  display.display(); //Sender alle endringer til skjermen

}



