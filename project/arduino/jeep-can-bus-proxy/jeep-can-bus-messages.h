// Can Id messages Jeep
#define CAN_POWER 0x000
#define CAN_BLINKERS 0x006
#define CAN_RADIO_MODE 0x09f
#define CAN_RADIO_CONTROLS 0x394 // Emitted? by the radio
#define CAN_VES_UNIT 0x3dd // Video unit
#define CAN_MULTI_SWITCH   0x11d
#define CAN_HEADLIGHTS
#define CAN_RADIO_SOUND_PROFILE 0x3D0
#define CAN_RADIO_POWER 0x416

/*
ID: 1A6 (steering wheel buttons)
00 00 (ready)
01 00 (menu up)
04 00 (menu down)
02 00 (menu right)
08 00 (music button)
10 00 (menu button)
20 00 (vol up)
80 00 (vol down)
40 00 (nav button)
Notes:
00 00 must follow other commands to "ready" the system
*/
//Basically the only thing is required to enable AUX is to regularly (0.5sec) send 8 bytes command:
// CAND ID 0x3dd, len 8, 03 00 00 00 00 00 00 00

// Setting time from the radio unconfirmed
// 0x0F0 message to SKREEM which keeps time and broadcasts it via 0x3EC
// 0x00 key state
// 0x210 lights status
// every 100ms on newer radio
//    canSend(0x000, keyState, 0x00, 0x00, 0x00, 0x00, 0x00); delay(5);                         //key position 00 = no key, 01 = key in, 41 = accessory, 81 = run, 21 = start
//    canSend(0x015, 85, 121, 6, 255, 0,0); delay(5);                                           //this needs to be here to turn the radio on initially. ECM data (voltage, + 2 other plots, FF, 00, 00)
//    canSend(0x1AF, 3, 131, 0, 192, 16, 44, 8, 0); delay(5);                                   //this needs to be here to keep the radio on or else it will shutoff after ~15 seconds (B0 changes from 3 to 1 to 3, the rest is static)
//    canSend(0x210, lightsDriving, lightsDashIntensity, 0x00, 0x00, 0x00, 0x00); delay(5);     //illumination information B0 = driving lights (0,1,2,3), B1 = dash intensity (00-C8)
//    canSend(0x3EC, timeH, timeM, timeS); delay(5);                                            //clock data, if this isn't here radio returns "no clock"
//    canSend(0x3A0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00); delay(5);                             //steering wheel button states from cluster

// Pressing steeringwheel radio ctrls
// 0x15
// 0x1B6
// 0x3A0
// 0x3D0 - sound profile Can Id, balance, fade, bass, mid, treble, ??, 0ff,
// 1- min/left/front/A-mitt/13-max/right/back
// 3D0 01C 00A 00A 00A 00A 00A 000 000 966
// 0x3F8
// 0x9F - from radio!

// Ids added when Jeep stereo is connected (still off)
// 0x1AB
// 0x1BB
// 0x416 (radio power)
// 0x394 - Radio setting / frequency etc
// ox3D0 - Sound profile
// 0x9F - from radio

// Ids added when Android radio is connected (on)
//0x1ab 1
//0x1bb 1
//0x394 1
//0x415 1



// TBD
// 0x2C0
// 0x3F1 - radio on / off state?
// 0x159


/* canhack.org Hardware/CAN Interface/List of CAN-B Decoded Messages/
rtgree01:

Steering Wheel Input Controls
id:3A0 byte:0 bit:0 -> Note Button
id:3A0 byte:0 bit:1 -> Vol Up Button
id:3A0 byte:0 bit:2 -> Vol Down Button
id:3A0 byte:0 bit:3 -> Up Arrow Button
id:3A0 byte:0 bit:4 -> Down Arrow Button
id:3A0 byte:0 bit:5 -> Right Arrow Button

SKREEM
id:012 {0x01, 0x02, 0x00, 0x40, 0x87, 0xa5} -> Lock Doors
id:012 {0x03, 0x02, 0x00, 0x40, 0x87, 0xa5} -> Unlock Doors
id:012 {0x05, 0x02, 0x00, 0x40, 0x87, 0xa5} -> Trunk Release
id:0x3EC - broadcast time!

Radio Status... One mode at a time
AM Radio Status
id:09F {0xm0, 0xnn, 0xoo, 0x07, 0xFF, 0xFF, 0xFF, 0x01}
m=preset # (mode)
0xnnoo = AM frequency
FM Radio Status
id:09F {0xm1, 0xnn, 0xoo, 0x07, 0xFF, 0xFF, 0xFF, 0x01}
m=preset #
0xnnoo = FM frequency * 10
CD Status
id:09F {0xm3, 0x00, 0xnn, 0x07, 0xoo, 0xpp, 0xqq, 0x01}
m=preset #
0xnn = track number
0xoo = hour
0xpp = minute
0xqq = second
Satellite Radio Status
id:09F {0xm4, 0x00, 0xnn, 0x07, 0xFF, 0xFF, 0xFF, 0x01}
m=preset #
0xnn = Satellite Channel

EVIC Display...
I think it only cares about AM or FM radio
AM Radio
id:394 {0xm0, 0xnn, 0xoo, 0x00, 0x00, 0x00}
m=preset #
0xnnoo = AM frequency
FM Radio
    CAN_RADIO.sendMsgBuf(0x1AF, 0, 0, 8, prof2, true); delay(CAN_DELAY_AFTER_SEND);    
id:394 {0xm1, 0xnn, 0xoo, 0x00, 0x00, 0x00}
m=preset #
0xnnoo = FM frequency * 10
Joko: 394,57,4,1F,0,0,0

Amplifier Settings
id:3D0 {0xVolume, 0xBalance, 0xFade, 0xBass, 0xMid, 0xTreble, 0x00}
Volume (0-39)
Balance (0-20)
Fade (0-20)
Bass (0-20) need to double check range
Mid (0-30) need to double check range
Treble (0-30) need to double check range
Joko: OK!

Radio Power
id:0x416 {0xFE, 0x16, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff}
Joko: 416,FD,1B,3F,FF,FF,FF,FF,FF

Satellite Radio Control
id:3B0 {0xmm, 0xstation, 0x00, 0x00, 0x00, 0x00}
mm=0x21
mm=0x23
mm=0x00
station = station number
Satellite Radio Status
id:1BD byte:0
value = 0x85 -> needs station initialization ??
id:1BD byte:1
value = channel number
Satellite Radio Genre
id:1BE
Satellite Radio Channel Name
id:1BF
Satellite Radio Text
id:3BD byte:0 bits:0-3 -> Line number (8 lines total) (Each line consists of 4 sub lines)
id:3BD byte:0 bits:4-7 -> Sub Line (clear the line when receiving sub line 1)
id:3BD bytes 1-7 -> Text
------
Linuxkidd:
Byte order is:
 0 1 2 3 4 5 6 7
0->MSB, 7->LSB
( Vice: 7 6 5 4 3 2 1 0 which is probably the way we 'should' address them )

1BD :: SDAR (Satellite Digital Radio) Status?
  Byte 0   : 0 -> Signal Good,  C -> No Signal, 8 -> Mute(during channel change)
  Byte 0.1 : 9 -> On, 5 -> Off, 1 -> Initial Power up?
  Byte 1   : Sirius Channel #
Remaining bytes change in sets.  Below are samples of observed data.
  Byte 2->3: 6D 83
  Byte 2->3: 86 83
  Byte 2->3: 69 94

43B :: SDAR More status? ( Maybe just presence )
  Observed variations:
Most common:       FD 3E 3F FF FF FF FF FF
Next most common:  FD 3E BF FF FF FF FF FF
  Remaining were only observed during buffer overflow state on the can232,
  and can probably be discarded.
And next most:     FE 3E 3F FF FF FF FF FF
Least common:      FE 3B 3F FF FF FF FF FF

1BE :: Genre ascii values ( Zero padded on right )
1BF :: Channel Name ascii values ( Zero padded on right )


3D0 :: Stereo Audio Settings ( Decimal )
    Byte 0 : Volume  00 -> 38  (Mute->Full Blast)
    Byte 1 : Balance 01 -> 19  (L->R)     10 => Center
    Byte 2 : Fade    01 -> 19  (F->R)     10 => Center
    Byte 3 : Bass    01 -> 19  (-9 -> +9) 10 => Center
    Byte 4 : Mid     01 -> 19  (-9 -> +9) 10 => Center
    Byte 5 : Treble  01 -> 19  (-9 -> +9) 10 => Center
    Byte 6 : On x13 Off x00 ????
    Joko: $ff when radio is attached, $00 when not (source indication?)

394 :: EVIC (Electronic vehicle information centre) :: FM and AM Mode
  Byte 0   : Same as 09F ( for 0.0 and 0.1 )
  Byte 1.0 : 0
  Byte 1.1,2 : Same as 09F
  Byte 3->5: 00 00 00

09F :: Radio :: Power up Logo
  Byte 0->7 : 07 20 00 00 FF FF FF CF

09F :: Radio :: AM Mode
  Byte 0.0 : Preset # ( 0 if not preset )
  Byte 0.1 : 0
  Byte 1.0 : 2
  Byte 1.1,2 : Chan #
  Byte 3->7: 01 FF FF FF 0F

09F :: Radio :: FM Mode
  Byte 0.0 : Preset # ( 0 if not preset )
  Byte 0.1 : 1
  Byte 1.0 : 2
  Byte 1.1,2 : Chan # * 10
  Byte 3->7: 01 FF FF FF 0F

09F :: Radio :: CD mode ( 6 disc MP3 Changer )
  Byte 0.0 : Disc #
  Byte 0.1 : 3
  Byte 1   : 20 ( Right halfbyte May be part of Track # if #>255 )
  Byte 2   : Track #
  Byte 3   : 3F ?
  Byte 4   : Hours
  Byte 5   : Minutes
  Byte 6   : Seconds
  Byte 7   : 0F ?

09F :: Radio :: Sirius Mode
  Byte 0.0 : Preset # ( 0 if not preset )
  Byte 0.1 : 4
  Byte 1   : x20
  Byte 2   : Chan #
  Byte 3->7: FF FF FF 0F
*/
