/*
 *  Remote Morse Code Server Encoder Via Ethernet.
 *
 *  Copyright (C) 2010 Efstathios Chatzikyriakidis (contact@efxa.org)
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 *  In order to compile the sketch you will need to hack the HashMap library.
 *
 *  Find the HashMap.h header and add this method to the HashMap class:
 *
 *    bool keyExists (hash key) {
 *      for (byte i = 0; i < size; i++) {
 *        if (hashMap[i].getHash () == key) {
 *          return true;
 *        }
 *      }
 *
 *      return false;
 *    }
 */

// include library for ethernet, networking.
#include <SPI.h>
#include <Ethernet.h>

// include library for map (hash implementation) data structure.
#include <HashMap.h>

const int ledPin = 8;   // the pin number of the led.
const int piezoPin = 7; // the pin number of the piezo.

// serial data rate transmission: bits per second.
const long serialBaud = 9600;

// enable this if you want to get messages on serial line.
const bool debugMode = true;

// morse code time slot delay time period (ms).
const long timeSlotDelay = 50;

// max number of morse code symbols.
const byte MORSE_CODE_SYMS = 53;

// hash table storage data structure for morse code symbols.
HashType<char, char *> hashArray[MORSE_CODE_SYMS];

// morse code map to handle the hash table storage.
HashMap<char, char *> morseCodes = HashMap<char, char *> (hashArray, MORSE_CODE_SYMS); 

// mac and ip addresses for the arduino ethernet shield.
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[]  = { 192, 168, 0, 1 };

// create morse code server and pass the port number.
EthernetServer server(5252);

// init international morse code symbols.
void
initMorseCodes() {
  morseCodes[0]('0', "-----"); // digit symbols.
  morseCodes[1]('1', ".----");
  morseCodes[2]('2', "..---");
  morseCodes[3]('3', "...--");
  morseCodes[4]('4', "....-");
  morseCodes[5]('5', ".....");
  morseCodes[6]('6', "-....");
  morseCodes[7]('7', "--...");
  morseCodes[8]('8', "---..");
  morseCodes[9]('9', "----.");

  morseCodes[10]('A', ".-"  ); // letter symbols.
  morseCodes[11]('B', "-...");
  morseCodes[12]('C', "-.-.");
  morseCodes[13]('D', "-.." );
  morseCodes[14]('E', "."   );
  morseCodes[15]('F', "..-.");
  morseCodes[16]('G', "--." );
  morseCodes[17]('H', "....");
  morseCodes[18]('I', ".."  );
  morseCodes[19]('J', ".---");
  morseCodes[20]('K', "-.-" );
  morseCodes[21]('L', ".-..");
  morseCodes[22]('M', "--"  );
  morseCodes[23]('N', "-."  );
  morseCodes[24]('O', "---" );
  morseCodes[25]('P', ".--.");
  morseCodes[26]('Q', "--.-");
  morseCodes[27]('R', ".-." );
  morseCodes[28]('S', "..." );
  morseCodes[29]('T', "-"   );
  morseCodes[30]('U', "..-" );
  morseCodes[31]('V', "...-");
  morseCodes[32]('W', ".--" );
  morseCodes[33]('X', "-..-");
  morseCodes[34]('Y', "-.--");
  morseCodes[35]('Z', "--..");

  // no wide characters support.

  // morseCodes[36]('Ä', ".-.-" ); // accented letter symbols.
  // morseCodes[37]('Ą', ".-.-" );
  // morseCodes[38]('Â', ".--.-");
  // morseCodes[39]('Á', ".--.-");
  // morseCodes[40]('À', ".--.-");
  // morseCodes[41]('Ç', "-.-..");
  // morseCodes[42]('Ć', "-.-..");
  // morseCodes[43]('É', "..-..");
  // morseCodes[44]('È', "..-..");
  // morseCodes[45]('Ę', "..-..");
  // morseCodes[46]('È', ".-..-");
  // morseCodes[47]('Ê', "-..-.");
  // morseCodes[48]('Ö', "---." );
  // morseCodes[49]('Ó', "---." );
  // morseCodes[50]('Ñ', "--.--");
  // morseCodes[51]('Ü', "..--" );
  // morseCodes[52]('Ş', "----" );
  // morseCodes[53]('Ź', "--.." );

  morseCodes[36]('/', "-..-."  ); // punctuation symbols.
  morseCodes[37]('?', "..--.." );
  morseCodes[38]('.', ".-.-.-" );
  morseCodes[39](',', "--..--" );
  morseCodes[40]('\'', ".----.");
  morseCodes[41]('!', "-.-.--" );
  morseCodes[42]('(', "-.--."  );
  morseCodes[43](')', "-.--.-" );
  morseCodes[44](':', "---..." );
  morseCodes[45](';', "-.-.-." );
  morseCodes[46]('"', ".-..-." );
  morseCodes[47]('_', "..--.-" );
  morseCodes[48]('-', "-....-" );
  morseCodes[49]('=', "-...-"  );
  morseCodes[50]('+', ".-.-."  );
  morseCodes[51]('$', "...-..-");
  morseCodes[52]('@', ".--.-." );
}

// play a beep sound for some time period.
void
playBeep(const long time) {
  // period of the wav (bigger means lower pitch).
  const long period = 100;

  // calculate the duration of the beep sound.
  const long beepDuration = (long) ((float) time / period * 1800);

  // light the led.
  digitalWrite(ledPin, HIGH);

  // play the beep sound (output the wave signal).
  for (int i = 0; i < beepDuration; i++) {
    digitalWrite(piezoPin, HIGH);
    delayMicroseconds(period);

    digitalWrite(piezoPin, LOW);
    delayMicroseconds(period);
  }

  // delay some time.
  delay(time);
}
 
// procedure for a single dot.
void
dot() {
  playBeep(timeSlotDelay);
}
 
// procedure for a single dash.
void
dash() {
  playBeep(timeSlotDelay * 3);
}

// silence for some time period.
void
silence(const long time) {
  digitalWrite(ledPin, LOW);
  delay(time);
}
 
// gap between dots and dashes.
void
timeSlotGap() {
  silence(timeSlotDelay);
}
 
// gap between symbols.
void
symbolGap() {
  silence(timeSlotDelay * 3);
}
 
// gap between words.
void
wordGap() {
  silence(timeSlotDelay * 7);
}

// try to handle a morse symbol.
void
handleMorseSymbol(const char * symbol) {
  // for each character (dash/dot) of the symbol.
  for (int i = 0; symbol[i] != '\0'; i++) {
    // try to handle a dash or dot.
    if (symbol[i] == '-') dash();
    if (symbol[i] == '.') dot();

    // gap delay time beetween dash/dot.
    timeSlotGap();
  }
}

// try to handle a character from a client.
void
handleClientChar(const char ch) {
  // if the character exists in the morse code character set.
  if (morseCodes.keyExists(ch)) {
    if (debugMode) {
      // show the morse code of the symbol.
      Serial.print(morseCodes.getValueOf(ch));
      Serial.print(' ');
    }

    // get the morse code of the symbol and handle it.
    handleMorseSymbol(morseCodes.getValueOf(ch));

    // gap delay between symbols.
    symbolGap();
  }
  else {
    // if it's a space character between words.
    if (ch == ' ') {
      if (debugMode)
        // show a space tag.
        Serial.print("[SPACE] ");

      // gap delay between words.
      wordGap();
    }
    else {
      if (debugMode)
        // show an unknown tag.
        Serial.print("[UNKNOWN] ");
    }
  }
}

// startup point entry (runs once).
void
setup() {
  // set led and piezo as outputs.
  pinMode(ledPin, OUTPUT); 
  pinMode(piezoPin, OUTPUT);

  // init international morse code symbols.
  initMorseCodes();

  // code for debug.
  if (debugMode) {
    // establish serial line.
    Serial.begin(serialBaud);

    // clear the serial buffer.
    while (Serial.available()) { Serial.read(); }

    // show international morse code.
    Serial.println("International Morse Codes:");
    morseCodes.debug();
  }

  // try to initialize the ethernet device.
  Ethernet.begin(mac, ip);

  // start the morse code server.
  server.begin();
}

// loop the main sketch.
void
loop() {
  // try to connect to a client with available data for reading.
  EthernetClient client = server.available();
  
  // if there is a connected client waiting, go for it.
  if (client) {
    // while the client is connected to the server.
    while (client.connected()) {
      // if the client has available data for reading.
      if (client.available())
        // read a character and try to handle it.
        handleClientChar(client.read());
      else
        // client's data has been handled, close the connection.
        client.stop();
    }
  }
}
