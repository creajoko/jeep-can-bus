#include <SoftwareSerial.h>
#include <SPI.h>
#include <stdio.h>
#include "mcp_can.h"
#include "jeep-can-bus-messages.h"

#define CAN_MODULE_CS_PIN 9
#define ANNOUNCE_PERIOD_MS 500
#define CAN_DELAY_AFTER_SEND 50
#define PROFILE_CHANGE_PERIOD_MS 5000

MCP_CAN CAN(CAN_MODULE_CS_PIN);

unsigned long lastAnnounce = millis();
unsigned long lastProfileChange = 0

// Messages needed to control Boston amp
unsigned char msg1[7] = {20, 10, 1, 10, 10, 10, 10};
unsigned char msg2[7] = {20, 10, 19, 10, 10, 10, 10};
unsigned char msgPowerOn[8] = {0xFD,0x1B,0x3F,0xFF,0xFF,0xFF,0xFF,0xFF};

const char compileDate[] = __DATE__ " " __TIME__;
/*
*/
void sendAnnouncements(profile) {
  if (profile) {}
    CAN.sendMsgBuf(CAN_RADIO_SOUND_PROFILE, 0, sizeof(profile), profile);
    delay(CAN_DELAY_AFTER_SEND);
  }
  CAN.sendMsgBuf(CAN_RADIO_POWER, 0, sizeof(msgPowerOn), msgPowerOn);
  delay(CAN_DELAY_AFTER_SEND);
}

void handleIncomingMessages() {
  if (CAN_MSGAVAIL != CAN.checkReceive())
    return;
  CAN.readMsgBuf(&len, buf);
  canId = CAN.getCanId();
  // Do nothing for now - add control via steering wheel buttons
}
/*
Setup
*/
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
/*
Main loop
*/
unsigned int canId = 0;
unsigned char len = 0;
unsigned char buf[8];
unsigned char newMode = 0;
unsigned char currProfile = 1;

void loop() {
    if (millis() > lastAnnounce + ANNOUNCE_PERIOD_MS) {
      lastAnnounce = millis();
      sendAnnouncements();
    }
   if (millis() > lastProfileChange + PROFILE_CHANGE_PERIOD_MS) {
        lastProfileChange = millis();
        if (currProfile == 1) {
          sendAnnouncements(msg2);
          currProfile = 2;
        } else {
          sendAnnouncements(msg1);
          currProfile = 1;
        }
   }
  handleIncomingMessages();
}
