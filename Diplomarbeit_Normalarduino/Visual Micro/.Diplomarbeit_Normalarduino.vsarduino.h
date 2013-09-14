#ifndef _VSARDUINO_H_
#define _VSARDUINO_H_
//Board = Arduino Duemilanove w/ ATmega328
#define __AVR_ATmega328P__
#define ARDUINO 105
#define __AVR__
#define F_CPU 16000000L
#define __cplusplus
#define __attribute__(x)
#define __inline__
#define __asm__(x)
#define __extension__
#define __ATTR_PURE__
#define __ATTR_CONST__
#define __inline__
#define __asm__ 
#define __volatile__
#define __builtin_va_list
#define __builtin_va_start
#define __builtin_va_end
#define __DOXYGEN__
#define prog_void
#define PGM_VOID_P int
#define NOINLINE __attribute__((noinline))

typedef unsigned char byte;
extern "C" void __cxa_pure_virtual() {;}

static unsigned long now ();
static void activityLed (byte on);
static void showNibble (byte nibble);
static void showByte (byte value);
static void addCh (char* msg, char c);
static void addInt (char* msg, word v);
static void saveConfig ();
static byte bandToFreq (byte band);
static void ookPulse(int on, int off);
static void fs20sendBits(word data, byte bits);
static void fs20cmd(word house, byte addr, byte cmd);
static void kakuSend(char addr, byte device, byte on);
static byte df_present ();
static void df_enable ();
static void df_disable ();
static byte df_xfer (byte cmd);
void df_command (byte cmd);
static void df_deselect ();
static void df_writeCmd (byte cmd);
void df_read (word block, word off, void* buf, word len);
void df_write (word block, const void* buf);
void df_flush ();
static void df_wipe ();
static void df_erase (word block);
static word df_wrap (word page);
static void df_saveBuf ();
static void df_append (const void* buf, byte len);
static void scanForLastSave ();
static void df_initialize ();
static void discardInput ();
static void df_dump ();
static word scanForMarker (word seqnum, long asof);
static void df_replay (word seqnum, long asof);
static void showString (PGM_P s);
static void showHelp ();
static void handleInput (char c);
void displayVersion(uint8_t newline );
//
//

#include "C:\Program Files (x86)\Arduino\hardware\arduino\variants\standard\pins_arduino.h" 
#include "C:\Program Files (x86)\Arduino\hardware\arduino\cores\arduino\arduino.h"
#include "C:\Users\Pablo\Documents\Arduino\ArduinoDiplomarbeit\Diplomarbeit_Normalarduino\Diplomarbeit_Normalarduino.ino"
#endif
