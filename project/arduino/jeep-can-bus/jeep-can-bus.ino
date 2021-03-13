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

unsigned long lastCheck = 0;
unsigned long lastAnnounce = millis();
unsigned long lastButtonPress = 0;
unsigned long lastDisplayRefresh = 0;

// Operational control
#define RADIOMODE_OTHER 0
#define RADIOMODE_AUX 1
//#define SCANNER
#define FILTERED_SCANNER

// Filtersetting max 20
//int can_id_filter[] = {0x015, 0x2C0, 0x3D0, 0x3F1, CAN_RADIO_MODE, 0x159, 0x1B6};
//unsigned int can_id_filter[] = {0x015};
int can_id_filter[] = { 0x015, 0x1B6, 0x3A0, 0x3D0, CAN_RADIO_MODE, CAN_RADIO_SOUND_PROFILE, 0x3F8, 0x1B6};
int filter_len = sizeof(can_id_filter);

// Messages
#define msgVesAuxModeLen 8
unsigned char msgVesAuxMode[8] = {3, 0, 0, 0, 0, 0, 0, 0};
#define msgPowerOnLen 6 // Emmited by car when power is on
unsigned char msgPowerOn[6] = {0x63, 0, 0, 0, 0, 0};

#define msgProfile1Len 7 // Emmited by car when power is on
unsigned char msgProfile1[7] = {CAN_RADIO_SOUND_PROFILE, 1, 1, 1, 1, 1, 0xFF};

unsigned char radioMode = RADIOMODE_OTHER;
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
void clearScr() { // Works w putty terminal
  Serial.write(27);
  Serial.print("[2J");
  Serial.write(27);
  Serial.print("[H");
}
  
void displayScr() {
  clearScr();
  for(int i=0;i<10;i++) {
    for(int j=0;j<10;j++) {
      char cell[4];
      sprintf(cell, " %03X",display[i][j]);
      Serial.print(cell);
      }
      Serial.println();
    }
    lastDisplayRefresh = millis();
  }
 
void sendAnnouncements() {
  //CAN.sendMsgBuf(CAN_RADIO_SOUND_PROFILE, 0, msgProfile1Len, msgProfile1);
  //delay(CAN_DELAY_AFTER_SEND);
}

unsigned int canId = 0;
unsigned char len = 0;
unsigned char buf[8];
unsigned char newMode = 0;

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
  // All messages on the c-bus
  Serial.print(canId, HEX);
  for (int i = 0; i < len; i++) {
    Serial.print(",");
    Serial.print(buf[i], HEX);
  }
  Serial.println();
  return;
#endif


#ifdef FILTERED_SCANNER
  // Targeted (can_id_filter) messages on the c-bus, csv
  int index = inArray(canId, can_id_filter);
  if(index<99) {
    display[index][0] = canId;
    for (int i = 1; i <= MESSAGE_LEN; i++) {
      display[index][i] = buf[i];
    }
    if (millis() > lastDisplayRefresh + DISPLAY_REFRESH_PERIOD) {
      displayScr();
    }
  }
  return;
#endif
}

void loop() {
  checkIncomingMessages();
}
