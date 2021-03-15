#include <SoftwareSerial.h>
#include <SPI.h>
#include <stdio.h>
#include "mcp_can.h"
#include "jeep-can-bus-messages.h"

#define CAN_MODULE_CS_PIN 9
#define ANNOUNCE_PERIOD_MS 1000
#define CAN_DELAY_AFTER_SEND 100

MCP_CAN CAN(CAN_MODULE_CS_PIN);

unsigned long lastCheck = 0;
unsigned long lastAnnounce = millis();

// Messages needed to control Boston amp
// Sound profile
#define msgProfile1Len 7
#define msgProfile1CanId 0x3D0
unsigned char msgProfile1[7] = {0x1A, 19, 19, 0x0B, 0x0B, 0x0B, 0xff};
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

void clearScr() {
  Serial.write(27);
  Serial.print("[2J");
  Serial.write(27);
  Serial.print("[H");
}
void setup() {
  Serial.begin(115200);
  clearScr();
  Serial.print("Jeep Boston Sound Systems amplifier controller");
  Serial.print("(c) Joakim Korling");
  Serial.println(compileDate);
  Serial.begin(115200);

  while (CAN_OK != CAN.begin(CAN_83K3BPS, MCP_16MHz)) {
    Serial.println("CAN init fail");
    delay(250);
  }
  Serial.println("CAN init ok");
}

void sendAnnouncements() {
  CAN.sendMsgBuf(msgProfile1CanId, 0, msgProfile1Len, msgProfile1);
  delay(CAN_DELAY_AFTER_SEND);

  CAN.sendMsgBuf(msgProfile2CanId, 0, msgProfile2Len, msgProfile2);
  delay(CAN_DELAY_AFTER_SEND);
  
  CAN.sendMsgBuf(msgProfile3CanId, 0, msgProfile3Len, msgProfile3);
  delay(CAN_DELAY_AFTER_SEND);

  CAN.sendMsgBuf(msgProfile4CanId, 0, msgProfile4Len, msgProfile4);
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
  // Do nothing for now - add control via steeringwheel buttons
}

void loop() {
    if (millis() > lastAnnounce + ANNOUNCE_PERIOD_MS) {
      lastAnnounce = millis();
      sendAnnouncements();
    }
  // handleIncomingMessages();
}
