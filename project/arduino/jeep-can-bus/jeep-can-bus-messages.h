// Can Id messages Jeep
#define CAN_POWER 0x000
#define CAN_BLINKERS 0x006
#define CAN_RADIO_MODE 0x09f
#define CAN_RADIO_CONTROLS 0x394 // Emitted by the radio when pressing buttons
#define CAN_VES_UNIT 0x3dd
#define CAN_MULTI_SWITCH   0x11d
#define CAN_HEADLIGHTS
#define CAN_RADIO_SOUND_PROFILE 0x3D0

// Pressing steeringwheel radio ctrls
// 0x15 - radio on/off?
// 0x1B6
// 0x3A0
// 0x3D0 - from radio, ifo about radio/cd etc, what frequency, track etc?
// 0x3F8
// 0x9F - from radio!

// Ids added when Jeep stereo is connected vs disconnected
// 0x1AB
// 0x416
// 0x394
// ox3D0
// 0x1BB
// 0x9F - from radio

// TBD
// 0x2C0
// 0x3F1 - radio on / off state or button press?
// 0x159