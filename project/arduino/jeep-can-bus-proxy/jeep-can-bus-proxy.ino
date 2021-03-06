#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <stdio.h>
#include <mcp_can.h>
#include <mcp2515_can.h>
#include "jeep-can-bus-messages.h"

#define MESSAGE_LEN 8
#define CAN_DELAY_AFTER_SEND 10
#define MAX_PROFILE_SEND_PERIOD 800
#define SIMULATION_MESSAGES_PERIOD 500

// Manage LED
#define LED_PERIOD 10000
#define LED_ON_TIME 500
#define LED_OFF_TIME 500
#define LED_FLASH_TIME 1000
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
#define SOUND_PROFILE_MSG_ID 0x3D0
unsigned char profile[7] = {20, 10, 10, 10, 10, 10, 0xFF};

unsigned char control_index = 0;
unsigned long boston_ctrl_button_pressed = 0;
unsigned long boston_ctrl_activated = 0;



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

  // LED signal
  pinMode(7, OUTPUT);

  Serial.println("CAN-bus proxy and Boston controller init ok");

  Serial.println("Loading sound profile from memory");
  if(EEPROM.read(0) != 255) {
    for(unsigned char i=0;i<7;i++) {
      profile[i] = EEPROM.read(i);
      Serial.print(profile[i]);
      Serial.print(", ");
    }
    Serial.println();
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
  //Serial.print("From jeep ");Serial.println(canId, HEX);
  // Apply rules
  if (canId == BOSTON_CTRL_MSG_ID and buf[0] != 0) {
    if (boston_ctrl_activated > 0) {
      Serial.println("Controller active");
      // Boston controller is active
      Serial.print("Control index: ");
      Serial.println(control_index);
      if(buf[0] == INCREASE_MSG) {
          unsigned char max = 19;
          if (control_index == 0) {
            max = 25; // Limit below max 39!
          }
          if (profile[control_index] < max) {
            Serial.println("Up");
            profile[control_index] += 1;
          }
          boston_ctrl_activated = millis();
      } else if(buf[0] == DECREASE_MSG) {
          if (profile[control_index] > 0) {
            Serial.println("Down");
            profile[control_index] -= 1;
          }
          boston_ctrl_activated = millis();
      } else if(buf[0] == COMMAND_MSG) {
          Serial.print("Change index to ");
          if (control_index < 5) {
            control_index += 1;
          } else {
            control_index = 0;
          }
          Serial.println(control_index);
          boston_ctrl_activated = millis();
      }
      // Store new profile
      Serial.print("Store new sound profile: ");
      for (unsigned int i=0; i<7; i++) {
        EEPROM.write(i, profile[i]);
        Serial.print(profile[i]);
        Serial.print(", ");
      }
      CAN_JEEP.sendMsgBuf(SOUND_PROFILE_MSG_ID, 0, 0, 7, profile);
      Serial.println("Profile sent to jeep");
      return;
    } else {
      // Boston controller is off, check if we shall activate
      if (buf[0] == COMMAND_MSG) {
        Serial.println("Controller activation command received");
        if (boston_ctrl_button_pressed > 0) {
          // We are in trigger activation mode
          if (millis() < boston_ctrl_button_pressed + ACTIVATION_PERIOD) {
            // Second press - activate controller, start index at volume!
            Serial.println("Activating controller");
            boston_ctrl_activated = millis();
            boston_ctrl_button_pressed = 0;
            control_index = 0;
            return;
          } else {
            // New press but too late to activate, Flush bufferand, clear timer
            Serial.println("Activation process cancelled");
            boston_ctrl_button_pressed = 0;
            boston_ctrl_activated = 0;
            unsigned char buffered_msg_array[2] = {buffered_msg, 0};
            buffered_msg = 0;
            CAN_RADIO.sendMsgBuf(canId, 0, 0, 2, buffered_msg_array);
            delay(CAN_DELAY_AFTER_SEND);
          }
        } else {
          // First command btn press; start trigger timer, store this msg, stop message from propagating
          Serial.println("Activation process started");
          boston_ctrl_button_pressed = millis();
          buffered_msg = buf[0];
          return;
        }
      }
    }
  }
  // Send off to radio
  CAN_RADIO.sendMsgBuf(canId, 0, 0, len, buf);
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
  Serial.print("From radio ");Serial.println(canId, HEX);
  if (canId == SOUND_PROFILE_MSG_ID) {
     // Replace radios profile with my profile
     CAN_JEEP.sendMsgBuf(SOUND_PROFILE_MSG_ID, 0, 0, 7, profile);
     return;
  }
  // Send off any all other messages to jeep
  CAN_JEEP.sendMsgBuf(canId, 0, 0, len, buf);
}

unsigned long simulation_time = 0;
unsigned char prof3[2] = {0x63, 0};
bool simulation_on = true;

void simulateRunningJeep() {
  if (simulation_on and millis() > simulation_time + SIMULATION_MESSAGES_PERIOD) {
    CAN_RADIO.sendMsgBuf(0x000, 0, 0, 2, prof3);
    Serial.print("Simulator to radio "); Serial.println(0x00, HEX);       
    simulation_time = millis();
  }
}

unsigned char led_flash_counter = 0;
void loop() {
  simulateRunningJeep();  
  manageMessagesFromRadio();
  manageMessagesFromJeep();  

  if (boston_ctrl_activated > 0 and millis() > boston_ctrl_activated + ACTIVE_PERIOD) {
    Serial.println("Controller cancelled by timeout");
    boston_ctrl_activated = 0;
    boston_ctrl_button_pressed = 0;
  }

  // LED signal controller state (index)
  if(boston_ctrl_activated > 0) {
    if(led_sequence_start == 0) {
      // Start
      led_sequence_start = millis();
      led_last_event = 0;
      led_flash_counter = 0;
      led_state = 0;
    }
    if(led_flash_counter <= control_index){
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
    return;
  }

  // Periodic slow LED flash, not in controller mode
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
