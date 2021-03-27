#include <SoftwareSerial.h>
#include <SPI.h>
#include <stdio.h>
#include <EEPROM>
#include "mcp_can.h"
#include "mcp2515_can.h"
#include "jeep-can-bus-messages.h"

#define MESSAGE_LEN 8
#define CAN_DELAY_AFTER_SEND 10

// Boston controller
// Left steering wheel button used to control sound
#define ACTIVATION_PERIOD 250
#define ACTIVE_PERIOD 10000
#define BOSTON_MIN_REFRESH_PERIOD 1000

#define BOSTON_CTRL_MSG_ID 0x3A0
#define COMMAND_MSG 0x01
#define INCREASE_MSG 0x02
#define DECREASE_MSG 0x04
#define balance 0
#define fade 1
#define bass 2
#define mid 3
#define treble 4
#define unknown 5
unsigned char profile = {20, 10, 10, 10, 10, 10, 0xFF }
#define MAX_VALUES = {39, 19, 19, 19, 19, 19}
unsigned char index = 0
unsigned long boston_ctrl_button_pressed = 0
unsigned long boston_ctrl_activated = 0
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

  if(EEPROM.read(0) != 0) {
    for(i=0;i<7;i++) {
      profile[i] = EEPROM.read(i);
      Serial.print(profile[i]);
      Serial.print(", ");
      Serial.println();
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
  if (canId == BOSTON_CTRL_MSG_ID) {
    if (boston_ctrl_activated > 0) {
      // Boston controller is on
      switch (buf[0]) {
        case INCREASE_MSG:
          if (profile[index] < MAX_VALUES[index]) {
            profile[index] += 1;
          }
          break;
        case DECREASE_MSG:
          if (profile[index] > 0) {
            profile[index] -= 1;
          }
          break;
        case COMMAND_MSG:
          if (index < 5) {
           index += 1;
          } else {
          index = 0;
          }
          break;
      }
      for (=0; i<7; i++) {
        EEPROM.write(i, profile[i]);
      }
      CAN_JEEP.sendMsgBuf(SOUND_PROFILE_MSG_ID, 0, 0, 7, profile, true)
    } else {
      // Boston controller is off
      if (buf[0] == COMMAND_MSG) {
        // This is a potential activation trigger
        if (boston_ctrl_button_pressed > 0) {
          if (millis() < boston_ctrl_button_pressed + ACTIVATION_PERIOD) {
            // Second press - activate controller
            boston_ctrl_activated = millis();
          } else {
            // New press but too late to activate
            // Send buffered msg and clear timer
            boston_ctrl_button_pressed = 0;
            CAN_RADIO.sendMsgBuf(canId, 0, 0, 2, [buffered_msg, 0], true);
          }
        } else {
          boston_ctrl_button_pressed = millis();
          buffered_msg = buf[0];
          return;
        }
      }
    } else {
    // Any other message - do not care
    }
  if (millis() > boston_ctrl_activated + ACTIVE_PERIOD) {
    boston_ctrl_activated = 0;
  }
  // Send off to radio
  Serial.println(CAN_RADIO.sendMsgBuf(canId, 0, 0, len, buf, true), HEX);
  delay(CAN_DELAY_AFTER_SEND);
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
     delay(CAN_DELAY_AFTER_SEND);
     return;
  }
  // Send off to jeep
  CAN_JEEP.sendMsgBuf(canId, 0, 0, len, buf, true);
}

void loop() {
  manageMessagesFromJeep();
  manageMessagesFromRadio();
}
