#include <SoftwareSerial.h>
#include <SPI.h>
#include <stdio.h>
#include "mcp_can.h"
#include "jeep-can-bus-messages.h"


#define CAN_MODULE_CS_PIN 9
#define CHECK_PERIOD_MS 200
#define ANNOUNCE_PERIOD_MS 500
#define BUTTON_PRESS_DEBOUNCE_MS 350
#define CAN_DELAY_AFTER_SEND 20
#define DISPLAY_REFRESH_PERIOD 500
#define MESSAGE_LEN 8
#define FIRST_ANNOUNCE_AFTER 10000

MCP_CAN CAN(CAN_MODULE_CS_PIN);

unsigned long filterStarted = millis();
unsigned long lastDisplayRefresh = 0;

// Operational control
//#define SCANNER
#define FILTERED_SCANNER
#define FILTER_PERIOD 10000

// Filtersetting max 20
//int can_id_filter[] = {0x015, 0x2C0, 0x3D0, 0x3F1, CAN_RADIO_MODE, 0x159, 0x1B6};
//unsigned int can_id_filter[] = {0x015};
int can_id_filter[] = { 0x015, 0x1B6, 0x3A0, 0x3D0, CAN_RADIO_MODE, CAN_RADIO_SOUND_PROFILE, 0x3F8, 0x1B6};
int filter_len = sizeof(can_id_filter);

const char compileDate[] = __DATE__ " " __TIME__;

void setup() {
  Serial.begin(115200);
  Serial.print("Jeep CAN-bus scanner");
  Serial.println(compileDate);
  Serial.begin(115200);

  while (CAN_OK != CAN.begin(CAN_83K3BPS, MCP_16MHz)) {
    Serial.println("CAN init fail");
    delay(250);
  }
  Serial.println("CAN init ok");
}

int display[20][20];
void clearScr() {
  Serial.write(27);
  Serial.print("[2J");
  Serial.write(27);
  Serial.print("[H");
}
  
void displayScr() {
  clearScr();
  for(int i=0;i<15;i++) {
    for(int j=0;j<15;j++) {
      char cell[4];
      if(j == MESSAGE_LEN+2) {
        sprintf(cell, " %03d",display[i][j]);
      } else {
        sprintf(cell, " %03X",display[i][j]);
      }
      Serial.print(cell);
      }
      Serial.println();
    }
    lastDisplayRefresh = millis();
  }

unsigned int canId = 0;
unsigned char len = 0;
unsigned char buf[8];

int inArray(int val, int arr[]) {
  for (int i = 0; i < filter_len; i++) {
    if (val == arr[i]) {
      return i;
    }
  }
  return 99;
}

void checkIncomingMessages() {
  if (CAN_MSGAVAIL != CAN.checkReceive())
    return;
  memset(buf, 0, 8);
  CAN.readMsgBuf(&len, buf);
  canId = CAN.getCanId();

#ifdef SCANNER
  // All messages on the c-bus to csv
  Serial.print(canId, HEX);
  for (int i = 0; i < len; i++) {
    Serial.print(",");
    Serial.print(buf[i], HEX);
  }
  Serial.println();
  return;
#endif

#ifdef FILTERED_SCANNER
  // Targeted (can_id_filter) messages on the c-bus, as csv
  int index = inArray(canId, can_id_filter);
  if(index<99) {
    display[index][0] = canId;
    for (int i = 0; i < MESSAGE_LEN; i++) {
      display[index][i+1] = buf[i];
    }
    // Add time since last message ms
    display[index][MESSAGE_LEN+2] = millis() - display[index][MESSAGE_LEN+2]
    if (millis() > lastDisplayRefresh + DISPLAY_REFRESH_PERIOD) {
      displayScr();
    }
  }
  return;
#endif
}

void loop() {
  checkIncomingMessages();
#ifdef FILTER_PERIOD
  if (millis() > filterStarted) {
    break;
  }
#endif
}
