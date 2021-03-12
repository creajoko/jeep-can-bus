#include <SoftwareSerial.h>
#include <SPI.h>
#include <stdio.h>
#include "mcp_can.h"


#define CAN_MODULE_CS_PIN 9
#define CHECK_PERIOD_MS 200
#define ANNOUNCE_PERIOD_MS 500
#define BUTTON_PRESS_DEBOUNCE_MS 350
#define CAN_DELAY_AFTER_SEND 20
#define DISPLAY_REFRESH_PERIOD 500

MCP_CAN CAN(CAN_MODULE_CS_PIN);

unsigned long lastCheck = 0;
unsigned long lastAnnounce = 0;
unsigned long lastButtonPress = 0;
unsigned long lastDisplayRefresh = 0;

// Operational control
#define RADIOMODE_OTHER 0
#define RADIOMODE_AUX 1
//#define SCANNER
#define FILTERED_SCANNER

// Can Id messages Jeep
#define CAN_POWER 0x000
#define CAN_BLINKERS 0x006
#define CAN_RADIO_MODE 0x09f
#define CAN_RADIO_CONTROLS 0x394 // Emitted by the radio 
#define CAN_VES_UNIT 0x3dd
#define CAN_MULTI_SWITCH   0x11d
#define CAN_HEADLIGHTS

// Pressing steeringwheel radio ctrls
// 0x15
// 0x1B6
// 0x3A0
// 0x3D0 - from radio?
// 0x3F8
// 0x9F - from radio!

// Ids added when Jeep stereo is connected (off)
// 0x1AB
// 0x416
// 0x394
// ox3D0
// 0x1BB
// 0x9F - from radio

// TBD
// 0x2C0
// 0x3F1
// 0x159

// Filtersetting max 20
int can_id_filter[] = {0x015, 0x2C0, 0x3D0, 0x3F1, CAN_RADIO_MODE, 0x159, 0x1B6};
//unsigned int can_id_filter[] = {0x015};

// Messages
#define msgVesAuxModeLen 8
unsigned char msgVesAuxMode[8] = {3, 0, 0, 0, 0, 0, 0, 0};
#define msgPowerOnLen 6 // Emmited by car when power is on
unsigned char msgPowerOn[6] = {0x63, 0, 0, 0, 0, 0};

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
  for(int i=0;i<20;i++) {
    for(int j=0;j<20;j++) {
      char cell[4];
      sprintf(cell, " %03X",display[i][j]);
      Serial.print(cell);
      }
      Serial.println();
    }
    lastDisplayRefresh = millis();
  }
 
void sendAnnouncements() {
#ifdef BENCH_MODE_ON
  // when on bench - send power on command to radio to enable it
  CAN.sendMsgBuf(CAN_POWER, 0, msgPowerOnLen, msgPowerOn);
  delay(CAN_DELAY_AFTER_SEND);
#endif
}

unsigned int canId = 0;
unsigned char len = 0;
unsigned char buf[8];
unsigned char newMode = 0;

bool inArray(int val, int arr[]) {
  for (int i = 0; i < sizeof(arr); i++) {
    if (val == arr[i]) {
      return i;
    }
  }
  return 99;
}



void checkIncomingMessages() {
  if (CAN_MSGAVAIL != CAN.checkReceive())
    return;
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
    for (int i = 1; i <= len; i++) {
      display[index][i] = buf[i];
    }
    if (millis() > lastDisplayRefresh + DISPLAY_REFRESH_PERIOD) {
      displayScr();
    }
  }
  return;
#endif

  switch (canId) {
    case CAN_RADIO_MODE:
      // Messages from the Radio
      Serial.print("CAN ID: ");
      Serial.print(canId, HEX);
      for (int i = 0; i < len; i++) {
        Serial.print(",");
        Serial.print(buf[i], HEX);
      }
      Serial.println();
      Serial.print("Radio mode: ");
      Serial.print(buf[0] & 0xF, HEX);
      Serial.print(":");
      Serial.print(radioMode);
      Serial.print(">");
      Serial.println(newMode);
      //  1-radio, 3-cd, 7-off
      newMode = ((buf[0] & 0xF) == 6) ? RADIOMODE_AUX : RADIOMODE_OTHER;

      if (radioMode != newMode) {
        radioMode = newMode;
        if (radioMode == RADIOMODE_AUX) {
          Serial.print("Radio Mode changed to AUX");
        } else {
          Serial.print("Radio Mode changed to something else");
        }
      }
      break;
    case CAN_RADIO_CONTROLS:
      // radio mode + buttons
      if (buf[3] > 0 && millis() > lastButtonPress + BUTTON_PRESS_DEBOUNCE_MS) { // something pressed
        lastButtonPress = millis();
        if (buf[3] & 1) { // seek right
          Serial.println("-- Seek >>");
        } else if (buf[3] & 2) { // seek left
          Serial.println("<< Seek --");
        } else if (buf[3] & 4) { // rw/ff right
          Serial.println("-- RW FF >>");
        } else if (buf[3] & 8) { // rw/ff left
          Serial.println("<< RW FF --");
        } else if (buf[3] & 0x20) { // RND/PTY
          Serial.println("[RND/PTY] - use as play/pause");
        }
      }

    default:
      break;
  }
}

void loop() {
  if (millis() > lastAnnounce + ANNOUNCE_PERIOD_MS) {
    lastAnnounce = millis();
    sendAnnouncements();
  }
  checkIncomingMessages();
}

// void setupFilters() {
//  CAN.init_Mask(0, 0, 0x3ff);
//  CAN.init_Mask(1, 0, 0x3ff);
//
//  CAN.init_Filt(0, 0, CAN_POWER);
//  CAN.init_Filt(1, 0, CAN_RADIO_MODE);
//  CAN.init_Filt(2, 0, CAN_RADIO_CONTROLS);
//  CAN.init_Filt(3, 0, CAN_BLINKERS);
//  CAN.init_Filt(4, 0, 0x08);
//  CAN.init_Filt(5, 0, 0x09);
//}
