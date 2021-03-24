#include <SoftwareSerial.h>
#include <SPI.h>
#include <stdio.h>
#include "mcp_can.h"
#include "mcp2515_can.h"
#include "jeep-can-bus-messages.h"

#define MESSAGE_LEN 8
#define CAN_DELAY_AFTER_SEND 20

const int SPI_CS_PIN_JEEP = 9;
const int SPI_CS_PIN_RADIO = 10;

mcp2515_can CAN_JEEP(SPI_CS_PIN_JEEP);
mcp2515_can CAN_RADIO(SPI_CS_PIN_RADIO);

const char compileDate[] = __DATE__ " " __TIME__;

void setup() {
  Serial.begin(115200);
  Serial.print("Jeep CAN-bus proxy");
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
  Serial.println("CAN PROXY init ok");
}

void manageMessagesFromJeep() {
  unsigned int canId;
  unsigned char len = 0;
  unsigned char buf[8];

  if (CAN_MSGAVAIL != CAN_JEEP.checkReceive())
    return;
  memset(buf, 0, 8);
  CAN_JEEP.readMsgBuf(&len, buf);
  canId = CAN_JEEP.getCanId();
  // Apply rules
  // Send off to radio
  CAN_RADIO.sendMsgBuf(canId, 0, 0, len, buf, true);
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
    Serial.println("3D0 message towards Jeep!!!!")
  }
  // Send off to jeep
  CAN_JEEP.sendMsgBuf(canId, 0, 0, len, buf, true);
  delay(CAN_DELAY_AFTER_SEND);
}

void loop() {
  manageMessagesFromJeep();
  manageMessagesFromRadio();
}
