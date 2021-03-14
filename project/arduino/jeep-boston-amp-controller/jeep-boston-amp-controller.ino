#include <SoftwareSerial.h>
#include <SPI.h>
#include <stdio.h>
#include "mcp_can.h"
#include "jeep-can-bus-messages.h"

#define CAN_MODULE_CS_PIN 9
#define ANNOUNCE_PERIOD_MS 1000
#define CAN_DELAY_AFTER_SEND 20

MCP_CAN CAN(CAN_MODULE_CS_PIN);

unsigned long lastCheck = 0;
unsigned long lastAnnounce = millis();

// Messages needed to control Boston amp
#define msgProfile1Len 7
#define msgProfile1CanId CAN_RADIO_SOUND_PROFILE
unsigned char msgProfile1[7] = {20, 20, 0x0B, 0x0B, 0x0B, 0x0B, 0xff};
// Last item: try 0x00, 0xff, 0x13, 0x1f

// Existing with jeep radio (unknown)
#define msgProfile2Len 6
#define msgProfile2CanId 0x1AB
unsigned char msgProfile2[6] = {3,0,0,0,0,0};

// Radio power 416	FD	1B	3F	FF	FF	FF	FF	FF
#define msgProfile3Len 8
#define msgProfile3CanId 0x416
unsigned char msgProfile3[8] = {0xFD,0x1B,0x3F,0xFF,0xFF,0xFF,0xFF,0xFF};

// Skipping 394 radio informs about frequency

// Existing with jeep radio (unknown) 1BB	0	0	0	0	0	0
#define msgProfile4Len 6
#define msgProfile4CanId 0x1BB
unsigned char msgProfile4[6] = {0,0,0,0,0,0};

// Skipping 095 radio mode AM/FM/CD

const char compileDate[] = __DATE__ " " __TIME__;

void setup() {
  Serial.begin(115200);
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
/*
  CAN.sendMsgBuf(msgProfile2CanId, 0, msgProfile2Len, msgProfile2);
  delay(CAN_DELAY_AFTER_SEND);
  CAN.sendMsgBuf(msgProfile3CanId, 0, msgProfile3Len, msgProfile3);
  delay(CAN_DELAY_AFTER_SEND);
  CAN.sendMsgBuf(msgProfile4CanId, 0, msgProfile4Len, msgProfile4);
  delay(CAN_DELAY_AFTER_SEND);
*/
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
  handleIncomingMessages();
}
