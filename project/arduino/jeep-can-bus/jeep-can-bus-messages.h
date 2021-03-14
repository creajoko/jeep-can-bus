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


// Pressing steeringwheel radio ctrls
// 0x15
// 0x1B6
// 0x3A0
// 0x3D0 - sound profile Can Id, balance, fade, bass, mid, treble, ??, 0ff,
// 1- min/left/front/A-mitt/13-max/right/back
// 0x3F8
// 0x9F - from radio!

// Ids added when Jeep stereo is connected (off)
// 0x1AB
// 0x416 (radio power)
// 0x394 - skum!
// ox3D0 - sound profile source?
// 0x1BB
// 0x9F - from radio

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
    Byte 6 : On x13 Off x00 ?

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
