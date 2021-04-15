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
  Serial.println("Jeep CAN-bus loop");
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
  pinMode(7, OUTPUT);
}

void manageMessagesFromJeepCard() {
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
  Serial.print("Jeep card recieved message ");
  Serial.println(canId, HEX);
  // Send off to radio
  //CAN_RADIO.sendMsgBuf(canId, 0, 0, len, buf, false);
}

void manageMessagesFromRadioCard() {
  unsigned int canId;
  unsigned char len = 0;
  unsigned char buf[8];  
  
  if (CAN_MSGAVAIL != CAN_RADIO.checkReceive())
    return;
  memset(buf, 0, 8);
  CAN_RADIO.readMsgBuf(&len, buf);
  canId = CAN_RADIO.getCanId();
  Serial.print("Radio card recieved message ");
  Serial.println(canId, HEX);
  // Apply rules
  //if (canId == 0x3D0) {
    //Serial.println("Send to Jeep, replace with correct profile");
    //CAN_JEEP.sendMsgBuf(SOUND_PROFILE_MSG_ID, 0, 0, 7, profile, true);
     //return;
  //}
  // Send off to jeep
  //Serial.print(canId);
  //Serial.println(" - Send to Jeep (but cycle stopped now)");
  //CAN_JEEP.sendMsgBuf(canId, 0, 0, len, buf, false);
}
unsigned long action = millis();
bool led = false;

void loop() {
  manageMessagesFromJeepCard();
  manageMessagesFromRadioCard();
  if (millis() > action + 100) {
    action = millis();
    if (led) {
      digitalWrite(7, LOW);
      led = not led;
      CAN_RADIO.sendMsgBuf(SOUND_PROFILE_MSG_ID, 0, 0, 7, profile, true);
      Serial.print("Inject message from radio card to Jeep card ");
      Serial.println(SOUND_PROFILE_MSG_ID, HEX);
    } else {
      digitalWrite(7, HIGH);
      led = not led;
      CAN_JEEP.sendMsgBuf(SOUND_PROFILE_MSG_ID, 0, 0, 7, profile, true);
      Serial.print("Inject message from Jeep card to radio card ");
      Serial.println(SOUND_PROFILE_MSG_ID, HEX);
    }
  }
}
