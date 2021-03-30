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

// Manage LED
#define LED_PERIOD 6000
#define LED_ON_TIME 400
#define LED_OFF_TIME 400
#define LED_FLASH_TIME 800
unsigned long led_sequence_start = 0;
unsigned long led_last_event = 0;
unsigned long led_last_flash_event = 0;
unsigned long led_flash_on = 0;
unsigned char led_flash_state = 0;
unsigned char led_state = 0;

// Boston controller
// Right steering wheel backside button used to control sound!
#define ACTIVATION_PERIOD 500
#define ACTIVE_PERIOD 10000

#define BOSTON_CTRL_MSG_ID 0x3A0
#define COMMAND_MSG 1
#define INCREASE_MSG 2
#define DECREASE_MSG 4
#define balance 0
#define fade 1
#define bass 2
#define mid 3
#define treble 4
#define unknown 5
unsigned char profile[7] = {20, 10, 10, 10, 10, 10, 0xFF};
unsigned char index = 0;
unsigned long boston_ctrl_button_pressed = 0;
unsigned long boston_ctrl_activated = 0;

#define SOUND_PROFILE_MSG_ID 0x3D0

const int SPI_CS_PIN_JEEP = 9;
const int SPI_CS_PIN_RADIO = 10;
unsigned char radio_status = 0;

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

  // LED signal
  pinMode(7, OUTPUT);

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
  if (canId == BOSTON_CTRL_MSG_ID and buf[0] != 0) {
    if (boston_ctrl_activated > 0) {
      Serial.println("BOSTON_CONTROLLER_ACTIVE");
      // Boston controller is on
      Serial.print("Control index: ");
      Serial.println(index);
      if(buf[0] == INCREASE_MSG) {
          unsigned char max = 19;
          if (index == 0) {
            max = 39;
          }
          if (profile[index] < max) {
            Serial.println("Up");
            profile[index] += 1;
            unsigned char temp_buf[3] = {0x55,0x70,0};
          }
          boston_ctrl_activated = millis();
      } else if(buf[0] == DECREASE_MSG) {
          if (profile[index] > 0) {
            Serial.println("Down");
            profile[index] -= 1;
            unsigned char temp_buf[5] = {0x44,0x6f,0x77,0x6e,0};
          }
          boston_ctrl_activated = millis();
      } else if(buf[0] == COMMAND_MSG) {
          if (index < 5) {
            Serial.println("Increase Index");
            index += 1;
          } else {
            index = 0;
          }
          Serial.println(index);
          boston_ctrl_activated = millis();
      }
      // Store new profile
      for (unsigned int i=0; i<7; i++) {
        EEPROM.write(i, profile[i]);
        Serial.print(profile[i]);
        Serial.print(", ");
      }
      CAN_JEEP.sendMsgBuf(SOUND_PROFILE_MSG_ID, 0, 0, 7, profile, true);
      profile_sent = millis();
      Serial.println("BOSTON_PROFILE_MSG sound sent");
      return;
    } else {
      // Boston controller is off, check if we shall activate
      if (buf[0] == COMMAND_MSG) {
        Serial.println("Trigger signal received");
        if (boston_ctrl_button_pressed > 0) {
          // We are in trigger activation mode
          if (millis() < boston_ctrl_button_pressed + ACTIVATION_PERIOD) {
            // Second press - activate controller, start at volume!
            Serial.println("Activated");
            boston_ctrl_activated = millis();
            boston_ctrl_button_pressed = 0;
            index = 0;
            return;
          } else {
            // New press but too late to activate
            // Send buffered msg and clear timer
            Serial.println("Too late - cancelling");
            boston_ctrl_button_pressed = 0;
            boston_ctrl_activated = 0;
            unsigned char buffered_msg_array[2] = {buffered_msg, 0};
            CAN_RADIO.sendMsgBuf(canId, 0, 0, 2, buffered_msg_array , true);
            delay(CAN_DELAY_AFTER_SEND);
            buffered_msg = 0;
          }
        } else {
          // First command btn press; start trigger timer, store this msg, stop message from propagating
          boston_ctrl_button_pressed = millis();
          buffered_msg = buf[0];
          return;
        }
      }
    }
  } else {
    // Any other message - do not care
  }
  if (boston_ctrl_activated > 0 and millis() > boston_ctrl_activated + ACTIVE_PERIOD) {
    Serial.println("Controller cancelled by timeout");
    boston_ctrl_activated = 0;
    boston_ctrl_button_pressed = 0;
  }
  if (boston_ctrl_button_pressed > 0 and millis() > boston_ctrl_button_pressed + ACTIVATION_PERIOD) {
    Serial.println("Second trigger not in time, sending original msg and cancelling");
    boston_ctrl_button_pressed = 0;
    unsigned char buffered_msg_array[2] = {buffered_msg, 0};
    CAN_RADIO.sendMsgBuf(canId, 0, 0, 2, buffered_msg_array, true);
    delay(CAN_DELAY_AFTER_SEND);
    buffered_msg = 0;
  }
  // Send off to radio if radio is active
  if (radio_status == 1) {}
    CAN_RADIO.sendMsgBuf(canId, 0, 0, len, buf, true);
}
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
     profile_sent = millis();
     return;
  }
  radio_status = 1;
  // Send off any message to jeep
  CAN_JEEP.sendMsgBuf(canId, 0, 0, len, buf, true);
}

unsigned char led_flash_counter = 0;

void loop() {
  manageMessagesFromJeep();
  manageMessagesFromRadio();

  if(boston_ctrl_activated > 0) {
    // LED Signal controller arctive
    if(led_sequence_start == 0) {
      // Start
      led_sequence_start = millis();
      led_last_event = 0;
      led_flash_counter = 0;
      led_state = 0;
    }
    if(led_flash_counter <= index){
      if(led_state == 0 and millis() > led_last_event + LED_OFF_TIME) {
        led_last_event = millis();
        led_state = 1;
        digitalWrite(7, HIGH);
      }
      if(led_state == 1 and millis() > led_last_event + LED_ON_TIME) {
        led_last_event = millis();
        led_state = 0;
        digitalWrite(7, LOW);
        led_flash_counter += 1;
      }
    } else {
        if(millis() > led_sequence_start + LED_PERIOD) {
          led_sequence_start = 0;
        }
    }
  } 
  if (millis() > led_last_flash_event + LED_PERIOD) {
    if(led_flash_state == 0) {
          led_flash_state = 1;
          led_flash_on = millis();
          digitalWrite(7, HIGH);
    }
    if(led_flash_state == 1 and millis() > led_flash_on + LED_FLASH_TIME) {
      led_flash_state = 0;
      digitalWrite(7, LOW);
      led_last_flash_event = millis();
    }
  }
}
