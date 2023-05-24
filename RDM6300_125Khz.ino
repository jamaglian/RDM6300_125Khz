//Links importantes
//https://github.com/pawl/Chinese-RFID-Access-Control-Library/blob/master/rfid.py
//https://www.pm-codeworks.de/pyrfid-en.html
//https://playground.arduino.cc/Main/RDM630RFIDReaderLibrary/
//https://github.com/jpswade/arduino-rfid-rdm6300
#include "Arduino.h"
#include "SoftwareSerial.h"
#include <string.h>

int rxPin = 4;
int txPin = 5;
int serialSpeed = 9600;
static const int STX = 2;
static const int ETX = 3;

byte _data[6];

int lastTag;

typedef enum {
  WAITING_FOR_STX,
  READING_DATA,
  DATA_VALID
} state;
state _s;

int nibble;
byte AsciiCharToNum(byte data);

SoftwareSerial rfid(rxPin, txPin);

void setup()
{
  Serial.begin(serialSpeed);
  rfid.begin(serialSpeed);
}

void loop()
{
  byte data[6];
  byte length;

  if (rfidAvailable()) {
    getData(data, length);
    int tag = getTag(data);
    //if (tag != lastTag) {
      char output[40];
      sprintf(output, "Decimal tag: %u", tag);
      Serial.println(output);
    //}
    lastTag = tag;
  }
}

bool rfidAvailable() { // Roda em loop
  if (rfid.available() > 0) //Se o leitor esta enviando
  {
    _s = dataParser(_s, rfid.read()); //Roda a funcao quando o leitor esta enviando enviando o atual state do codigo
    return (_s == DATA_VALID); //Retorna true se o código for valido
  }
  return false;
}

state dataParser(state s, byte c) { //Roda em loop se o cartao esta sendo lido
  switch (s) { 
    case WAITING_FOR_STX:
    case DATA_VALID:
      if (c == STX) {
        nibble = -1;
        return READING_DATA;
      }
      break;
    case READING_DATA:
      if (++nibble < 12) {
        //Serial.println(nibble);
        _data[nibble >> 1] = ((nibble & 0x1) == 0 ? AsciiCharToNum(c) << 4 : _data[nibble>>1] + AsciiCharToNum(c));
        //Serial.print(nibble >> 1);
        //Serial.print(": ");
        //Serial.println(_data[nibble >> 1]);
        return READING_DATA;
      }
      if (c != ETX) { // Expected end character, but got something else.
        return WAITING_FOR_STX;
      }
      for (int i = 0; i < 5; i++) {
        _data[5] ^= _data[i];
      }
      if (_data[5] != 0) { // Checksum doesn't match.
        return WAITING_FOR_STX;
      }
      return DATA_VALID;
    default:
      return WAITING_FOR_STX;
  }
  return WAITING_FOR_STX;
}

int getTag(byte* data) {
  // Concatenate the bytes in the data array to one long rendered as a decimal.
  int thisTag =
    ((int)data[1] << 24) +
    ((int)data[2] << 16) +
    ((int)data[3] << 8) +
    data[4];
   Serial.println(data[2]);
  return thisTag;
}

void getData(byte* data, byte& length) {
  length = sizeof(_data);
  memcpy(data, _data, sizeof(_data));
}

byte AsciiCharToNum(byte data) {
  return (data > '9' ? data - '0' - 7 : data - '0');
}
