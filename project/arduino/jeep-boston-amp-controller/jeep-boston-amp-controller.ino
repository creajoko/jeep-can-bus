#include <SoftwareSerial.h>
#include <SPI.h>
#include <stdio.h>
#include "mcp_can.h"
#include "jeep-can-bus-messages.h"

#define CAN_MODULE_CS_PIN 9
#define ANNOUNCE_PERIOD_MS 500
#define CAN_DELAY_AFTER_SEND 200

MCP_CAN CAN(CAN_MODULE_CS_PIN);

unsigned long lastAnnounce = millis();

// Messages needed to control Boston amp
#define msgProfile1Len 7
unsigned char msgProfile1[7] = {0x12, 0x05, 0x05, 0x05, 0x05, 0x05, 0xFF};

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
  CAN.sendMsgBuf(CAN_RADIO_SOUND_PROFILE, 0, msgProfile1Len, msgProfile1);
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
  handleIncomingMessages();
}
