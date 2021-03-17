#include <SoftwareSerial.h>
#include <SPI.h>
#include <stdio.h>
#include "mcp_can.h"
#include "jeep-can-bus-messages.h"
#include "mcp2515_can.h"

#define ANNOUNCE_PERIOD_MS 1000
#define CAN_DELAY_AFTER_SEND 100

const int SPI_CS_PIN = 9;
const int CAN_INT_PIN = 2;

mcp2515_can CAN(SPI_CS_PIN); // Set CS pin

unsigned long lastCheck = 0;
unsigned long lastAnnounce = millis();

// Messages needed to control Boston amp
// Sound profile
#define msgProfile1Len 7
#define msgProfile1CanId 0x3D0
unsigned char msgProfile1[7] = {20, 19, 10, 0x0B, 0x0B, 0x0B, 0xff};
// vol 0-38, övriga 1-19

// 3F1 Radio On 0 / Off 1
#define msgProfile2Len 2
#define msgProfile2CanId 0x3F1
unsigned char msgProfile2[2] = {0x41,0};

// Radio start up status 8 started
#define msgProfile3Len 8
#define msgProfile3CanId 0x159
unsigned char msgProfile3[8] = {0x08,0xFF,0xFF,0xFF,0x01,0xFF,0x00,0x08};

// Skipping 394 radio informs about frequency on display EVIC

// Radio mode
#define msgProfile4Len 8
#define msgProfile4CanId 0x09F
unsigned char msgProfile4[8] = {0x01,0x13,0x9D,0x01,0xFF,0xFF,0xFF,0x11};

const char compileDate[] = __DATE__ " " __TIME__;

void setup() {
  Serial.begin(115200);
  Serial.print("Jeep Boston Sound Systems amplifier controller");
  Serial.print("(c) Joakim Korling");
  Serial.println(compileDate);

  while (CAN_OK != CAN.begin(CAN_83K3BPS, MCP_16MHz)) {
    Serial.println("CAN init fail");
    delay(250);
  }
  CAN.setMode(MODE_NORMAL);
  Serial.println("CAN init ok");
}

void printSendStatus(unsigned int can_id, unsigned char sndStat) {
  Serial.print("Message: ");
  Serial.print(can_id, HEX);
  if(sndStat == CAN_OK){
    Serial.println("-OK");
  } else {
    Serial.println("-Failed");
  }
}

void sendAnnouncements() {
  //rtr bit1 bryr sig ej, 0-reagerar men går tillbaka
  printSendStatus(
    msgProfile1CanId,
    CAN.sendMsgBuf(msgProfile1CanId, 0, 0, msgProfile1Len, msgProfile1, true)
  );
  delay(CAN_DELAY_AFTER_SEND);
  printSendStatus(
    msgProfile2CanId,
    CAN.sendMsgBuf(msgProfile2CanId, 0, 0, msgProfile2Len, msgProfile2, true)
  );
  delay(CAN_DELAY_AFTER_SEND);

  CAN.sendMsgBuf(msgProfile3CanId, 0,0,  msgProfile3Len, msgProfile3, true);
  delay(CAN_DELAY_AFTER_SEND);

  CAN.sendMsgBuf(msgProfile4CanId, 0,0,  msgProfile4Len, msgProfile4, true);
  delay(CAN_DELAY_AFTER_SEND);
  
}

unsigned int canId = 0;
unsigned char len = 0;
unsigned char buf[8];
unsigned char newMode = 0;

void handleIncomingMessages() {
  if (CAN_MSGAVAIL != CAN.checkReceive())
    return;
  CAN.readMsgBuf(&len, buf);
  canId = CAN.getCanId();
  // Do nothing for now - add control via steering wheel buttons
}

void loop() {
    if (millis() > lastAnnounce + ANNOUNCE_PERIOD_MS) {
      lastAnnounce = millis();
      sendAnnouncements();
    }
    handleIncomingMessages();
}
