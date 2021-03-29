#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <stdio.h>
#include "mcp_can.h"
#include "mcp2515_can.h"
#include "jeep-can-bus-messages.h"

#define MESSAGE_LEN 8
#define CAN_DELAY_AFTER_SEND 10
#define MAX_PROFILE_SEND_PERIOD 800
unsigned long profile_sent = 0;


unsigned char profile[7] = {20, 10, 10, 10, 10, 10, 0xFF};

#define SOUND_PROFILE_MSG_ID 0x3D0

const int SPI_CS_PIN_JEEP = 9;
const int SPI_CS_PIN_RADIO = 10;

mcp2515_can CAN_JEEP(SPI_CS_PIN_JEEP);
mcp2515_can CAN_RADIO(SPI_CS_PIN_RADIO);

const char compileDate[] = __DATE__ " " __TIME__;

void setup() {
  Serial.begin(115200);
  Serial.println("Jeep CAN-bus proxy");
  Serial.println(compileDate);
  Serial.begin(115200);

  while (CAN_OK != CAN_JEEP.begin(CAN_83K3BPS, MCP_16MHz)) {
    Serial.println("CAN_JEEP init fail");
    delay(250);
  }
  while (CAN_OK != CAN_RADIO.begin(CAN_83K3BPS, MCP_16MHz)) {
      Serial.println("CAN_RADIO init fail");
      delay(250);
  }
  CAN_RADIO.setMode(MODE_NORMAL);
  CAN_JEEP.setMode(MODE_NORMAL);
  Serial.println("CAN-bus proxy and Boston controller init ok");

  if(EEPROM.read(0) != 255) {
    for(unsigned char i=0;i<7;i++) {
      profile[i] = EEPROM.read(i);
      Serial.print(profile[i]);
    }
  }
}

void manageMessagesFromJeep() {
  unsigned int canId;
  unsigned char len = 0;
  unsigned char buf[8];
  unsigned char buffered_msg = 0;
  
  if (CAN_MSGAVAIL != CAN_JEEP.checkReceive())
    return;
  memset(buf, 0, 8);
  CAN_JEEP.readMsgBuf(&len, buf);
  canId = CAN_JEEP.getCanId();
  // Apply rules
  // Send off to radio
  CAN_RADIO.sendMsgBuf(canId, 0, 0, len, buf, true);
}

void manageMessagesFromRadio() {
  unsigned int canId;
  unsigned char len = 0;
  unsigned char buf[8];  
  
  if (CAN_MSGAVAIL != CAN_RADIO.checkReceive())
    return;
  memset(buf, 0, 8);
  CAN_RADIO.readMsgBuf(&len, buf);
  canId = CAN_RADIO.getCanId();
  // Apply rules
  if (canId == 0x3D0) {
     CAN_JEEP.sendMsgBuf(SOUND_PROFILE_MSG_ID, 0, 0, 7, profile, true);
     return;
  }
  // Send off to jeep
  CAN_JEEP.sendMsgBuf(canId, 0, 0, len, buf, true);
}

void loop() {
  manageMessagesFromJeep();
  manageMessagesFromRadio();
  if(profile_sent + MAX_PROFILE_SEND_PERIOD < millis()) {
      Serial.println("Sending profile");
     CAN_JEEP.sendMsgBuf(SOUND_PROFILE_MSG_ID, 0, 0, 7, profile, true);
     profile_sent = millis();
  }
}
