#include <cubeServer.h>

#include <JeeLib.h>
#include <RF12sio.h>
#include <RF12.h>
#include <util/crc16.h>
#include <util/parity.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>


// ATtiny's only support outbound serial @ 38400 baud, and no DataFlash logging

/*long paultime1;
 long paultime2;
 long paultimesp;
 */

int rfmids = 2;
int rfmid_backup_strat;
String serialinput;
long timestart = 0;
int leval = 1;

#if defined(__AVR_ATtiny84__) ||defined(__AVR_ATtiny44__)
#define SERIAL_BAUD 38400
#else
#define SERIAL_BAUD 57600

#define LICHT_PIN	4	//Licht ein und ausschalten!

#define DATAFLASH 1 // check for presence of DataFlash memory on JeeLink
#define FLASH_MBIT  16  // support for various dataflash sizes: 4/8/16 Mbit

#define LED_PIN   9 // activity LED, comment out to disable

#endif

#define COLLECT 0x20 // collect mode, i.e. pass incoming without sending acks

static unsigned long now () {
  // FIXME 49-day overflow
  return millis() / 1000;
}

static void activityLed (byte on) {
#ifdef LED_PIN
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, !on);
#endif
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// RF12 configuration setup code

typedef struct {
  byte nodeId;
  byte group;
  char msg[RF12_EEPROM_SIZE-4];
  word crc;
} 
RF12Config;

static RF12Config config;

static char cmd;
static byte value, stack[RF12_MAXDATA+4], top, sendLen, dest, quiet;
static byte testbuf[RF12_MAXDATA], testCounter, useHex;

void sendIdToNewDevice(char id, char ){
}

String read() {
  int bytes_read = 0;
  String cmd = "";
  bool valid = false;

  while (!valid && Serial.available()) {
    char in = Serial.read();
    if (in == ';') {
      valid = true;
    } 
    else {
      cmd += (char) in;
    }
    bytes_read = bytes_read + 1;
    delay(2);
  }

  return (valid) ? cmd : "err";
}

static void addCh (char* msg, char c) {
  byte n = strlen(msg);
  msg[n] = c;
}

static void addInt (char* msg, word v) {
  if (v >= 10)
    addInt(msg, v / 10);
  addCh(msg, '0' + v % 10);
}

static void saveConfig () {
  // set up a nice config string to be shown on startup
  memset(config.msg, 0, sizeof config.msg);
  strcpy(config.msg, " ");

  byte id = config.nodeId & 0x1F;
  addCh(config.msg, '@' + id);
  strcat(config.msg, " i");
  addInt(config.msg, id);
  if (config.nodeId & COLLECT)
    addCh(config.msg, '*');

  strcat(config.msg, " g");
  addInt(config.msg, config.group);

  strcat(config.msg, " @ ");
  static word bands[4] = { 
    315, 433, 868, 915     };
  word band = config.nodeId >> 6;
  addInt(config.msg, bands[band]);
  strcat(config.msg, " MHz ");

  config.crc = ~0;
  for (byte i = 0; i < sizeof config - 2; ++i)
    config.crc = _crc16_update(config.crc, ((byte*) &config)[i]);

  // save to EEPROM
  for (byte i = 0; i < sizeof config; ++i) {
    byte b = ((byte*) &config)[i];
    eeprom_write_byte(RF12_EEPROM_ADDR + i, b);
  }

  if (!rf12_config())
    Serial.println("config save failed");
}

static byte bandToFreq (byte band) {
  return band == 8 ? RF12_868MHZ : band == 9 ? RF12_915MHZ : RF12_433MHZ;
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  activityLed(0);

  config.nodeId = 0x5E; // 433 MHz, node 30
  config.group = 0xD4;  // default group 212
  saveConfig();

  Serial.println(";");
}

void loop() {
  String serialinput;
  // if (Serial.available())
  // handleInput(Serial.read());


  serialinput = read();
  if (rf12_recvDone()) {
    /*byte n = rf12_len;
    if (rf12_crc == 0)
      Serial.print("");
    else {
      return;
    }*/


    if((int) rf12_data[0] == 11 && (int) rf12_data[1] == 11 && (int) rf12_data[2] == 11){

      static byte send_newid[70] =  {
        RF12_433MHZ, 212, 2, 111, 111, 111      };
        
      rf12_sendStart(send_newid[2], send_newid+3, 3);
      /*rf12_sendWait(0);*/

    }
    if( (int) rf12_data[0] == 1 && (int) rf12_data[1] == 1 && (int) rf12_data[2] == 1 ){   
      Serial.println("REQUEST:ID;");
    }
    if( (int) rf12_data[0] == 15 && (int) rf12_data[1] == 15 && (int) rf12_data[2] == 15 ){   
      Serial.print("SUCCESS:IDSET|ID=");
      int id = (int) rf12_data[3];
      Serial.print(id);
      Serial.println(";");
    }
    if( (int) rf12_data[0] == 20 && (int) rf12_data[1] == 20 && (int) rf12_data[2] == 20 ){   
      Serial.print("SUCCESS:VALSET|ID=");
      int id = (int)rf12_data[4];
      Serial.print(id);
      Serial.print("|VALUE=");
      int val = (int)rf12_data[3];
      Serial.print(val);
      Serial.println(";");
    }
    if((int) rf12_data[0] == 40 && (int) rf12_data[1] == 40 && (int) rf12_data[2] == 40){
      Serial.print("SUCCESS:VALUE|ID=");
      int id = (int)rf12_data[5];
      Serial.print(id);
      Serial.print("|VALUE=");
      int val_integer = (int)rf12_data[3];
      int val_pointnumber = (int) rf12_data[4];
      Serial.print(val_integer);
      Serial.print(".");
      Serial.print(val_pointnumber);
      Serial.println(";");
    }

    /*if (rf12_crc == 0) {
      activityLed(1);

      if (RF12_WANTS_ACK && (config.nodeId & COLLECT) == 0) {
        rf12_sendStart(RF12_ACK_REPLY, 0, 0);
      }

      activityLed(0);
    }*/
  }

  if(serialinput.substring(0,6) == "B!R:ID"){//B!R:ID|ID=X;
    //SEND ID TO NODE 99
    char target_id[5];
    target_id[0] = serialinput[10];
    int ind = 11;
    int length = serialinput.length();

    while(ind < length){
      target_id[ind-10] = serialinput[ind];
      ind++; 
    }

    int t_id = atoi(target_id);
    byte send_newid[70] =  {
      RF12_433MHZ, 212, 2, 200, 200, 200, t_id        };
      
    rf12_sendStart(send_newid[2], send_newid+3, 4);
    /*rf12_sendWait(0);*/
    for(int i = 0; i<5;i++){
      target_id[i] = NULL;
      t_id = NULL;
    }

  }
  else if(serialinput.substring(0,11) == "C!E:COMMAND"){          //C!E:COMMAND|ID=X|VALUE=Y;
   
    
    char target_id[5];
    target_id[0] = serialinput[15];
    int index = 16;
    int length = serialinput.length();

    int diff = 124;

    while(serialinput[index] != (char) diff){
      target_id[index - 15] = serialinput[index]; 
      index++;
    }

    int t_id = atoi(target_id);

    int pos = index - 16;

    char target_value[5];
    target_value[0] = serialinput[23 + pos];
    index = 24 + pos;

    while(index < length){
      target_value[index - (23 + pos)] = serialinput[index]; 
      index++;
    }

    int t_val = atoi(target_value);
    byte send_newid[70] =  {
      RF12_433MHZ, 212, 2, 205, 205, 205, t_id, t_val};
      
    rf12_sendStart(send_newid[2], send_newid+3, 5);
    /*rf12_sendWait(0);*/

    for(int i = 0; i<5;i++){
      target_id[i] = NULL;
      target_value[i] = NULL;
      t_id = NULL;
      t_val = NULL;
    }

  }

  else if(serialinput.substring(0,10) == "W!R:UPDATE"){//W!R:UPDATE|ID=X
    char target_id[5];
    target_id[0] = serialinput[14];
    int ind = 15;
    int length = serialinput.length();

    while(ind < length){
      target_id[ind-14] = serialinput[ind];
      ind++; 
    }

    int t_id = atoi(target_id);
    byte send_newid[70] =  {
      RF12_433MHZ, 212, 2, 205, 205, 205, t_id, 1};
      
    rf12_sendStart(send_newid[2], send_newid+3, 5);
    /*rf12_sendWait(0);*/
    for(int i = 0; i<5;i++){
      target_id[i] = NULL;
      t_id = NULL;
    }

  }        

  /*if (cmd && rf12_canSend()) {
    activityLed(1);
    byte header = cmd == 'a' ? RF12_HDR_ACK : 0;
    if (dest)
      header |= RF12_HDR_DST | dest;
    rf12_sendStart(header, testbuf, sendLen);
    cmd = 0;

    activityLed(0);
  }*/
  /*long timenow = millis();
  if(timenow >= timestart+100){
   
    byte send_newid[70] =  {
      RF12_433MHZ, 212, 2, 205, 205, 205, 34, leval};
    //if(rf12_canSend()){
      Serial.print("\n Timenow: ");
    Serial.print(timenow);
    Serial.print("\t Timestart: ");
    Serial.print(timestart);
    rf12_sendStart(send_newid[2], send_newid+3, 5);
    leval = !leval;
    //rf12_sendWait(1);
    //}
    
    
    timestart = timenow;
  }*/

}



