
// Standalone UPDI programmer from SD CArd using ESP32 - designed for this board: https://espprogrammerdocs.readthedocs.io/en/latest/index.html
#include "Adafruit_AVRProg.h"// !!! MY FORK MUST BE USED !!! https://github.com/krdarrah/Adafruit_AVRProg

Adafruit_AVRProg avrprog = Adafruit_AVRProg();

#include <SPI.h>
#include "SD.h"

//PINS
const int SD_CS_pin = 25;
const int redLED_pin = 22;
const int greenLED_pin = 32;
const int yellowLED_pin = 33;
const int startProgramming_pin = 21;
const int avrPowerPin = 14;

SPIClass spiSD(VSPI);

//functions in the sketch
void initSDcard();
bool startFlashing();
void getFileNames();

//globals
const char hexKey[] = ".hex";
const char hiddenFileKey[] = "._";//on mac, we may find duplicates that start with this, so we filter out
char hexFileName[100] = {NULL};
unsigned long greenLEDflashStart;

/* for reference when uploaded from IDE
/Users/kevindarrah/Library/Arduino15/packages/megaTinyCore/tools/python3/3.7.2-post1/python3 -u /Users/kevindarrah/Library/Arduino15/packages/megaTinyCore/hardware/megaavr/2.5.11/tools/prog.py -t uart -u /dev/cu.usbserial-DA00XJ7V -b 230400 -d attiny1614 --fuses 2:0x02 6:0x04 8:0x00 -f/var/folders/gv/dqd77lfs72xgzhcbwrp3f6vc0000gn/T/arduino_build_634928/strandtest1614.ino.hex -a write -v 
*/
void setup() {
  Serial.begin(115200);
  pinMode(startProgramming_pin, INPUT);
  pinMode(redLED_pin, OUTPUT);
  pinMode(greenLED_pin, OUTPUT);
  pinMode(yellowLED_pin, OUTPUT);
  digitalWrite(redLED_pin, HIGH);//on while booting

  Serial.println("starting updi programmer");
  initSDcard();

  avrprog.setUPDI(&Serial2, 230400);//this baud rate is fixed in code now, TODO figure out how to get faster bauds... 
  //avrprog.setUPDI(&Serial2, 230400, avrPowerPin); // this will cycle power on the pin furing resets
  avrprog.setProgramLED(yellowLED_pin);
  avrprog.setErrorLED(redLED_pin);
  pinMode(avrPowerPin, OUTPUT);
  digitalWrite(avrPowerPin, HIGH);//will be low when programmer is reset to provide power cycle
  digitalWrite(redLED_pin, LOW);//all good now
  greenLEDflashStart = millis();
}

void loop() {
  if (millis() - greenLEDflashStart > 500) {
    greenLEDflashStart = millis();
    digitalWrite(greenLED_pin, !digitalRead(greenLED_pin));//flashgreen while waiting
  }

  if (!digitalRead(startProgramming_pin)) {
    digitalWrite(greenLED_pin, LOW);
    digitalWrite(yellowLED_pin, HIGH);
    unsigned startTime = millis();
    if (avrprog.targetPower(true)) {
      //avrprog.error("Failed to connect to target");

      //avrprog.UPDIunlock();

      Serial.print(F("\nReading signature: "));
      uint16_t signature = avrprog.readSignature();
      Serial.println(signature, HEX);
      digitalWrite(yellowLED_pin, !digitalRead(yellowLED_pin));
      ////
      //          Serial.print(F("\nReading fuses: "));
      //          uint8_t fuses[10];
      //          if (! avrprog.readFuses(fuses, 10)) {
      //            avrprog.error(F("Couldn't read fuses"));
      //          }
      //          for (uint8_t i = 0; i < 10; i++) {
      //            Serial.print("0x"); Serial.print(fuses[i], HEX); Serial.print(", ");
      //          }
      //          Serial.println();

      //TODO would be to check signature

      Serial.print(F(" - Erasing chip..."));
      if (! avrprog.eraseChip()) {
        digitalWrite(greenLED_pin, LOW);
        digitalWrite(yellowLED_pin, LOW);
        digitalWrite(redLED_pin, HIGH);//error
        avrprog.error(F("Failed to erase flash"));
        while (1) {
          //
        }
      }
      Serial.println("writing fuses");
      digitalWrite(yellowLED_pin, !digitalRead(yellowLED_pin));

      //for the 1614  TODO - add these fuses to a config file
      avrprog.programFuse(0x02, 2); //data, fuse number
      digitalWrite(yellowLED_pin, !digitalRead(yellowLED_pin));
      avrprog.programFuse(0x04, 6); //data, fuse number
      digitalWrite(yellowLED_pin, !digitalRead(yellowLED_pin));
      avrprog.programFuse(0x00, 8); //data, fuse number
      // on the ATTINY1614, we have 64B per page, so 32pages at a time, see line 518 in AVRprog.cpp
      //each line can have a max of 16B, so 64B would be 4lines, but there's other bytes in the line, 45B total
      //array size is then 4lines * 32pages * 45B = 5760, so in future, would be good to calculate these things aut
      //5760 is a fixed page size, so make sure this calculation always equals 5760

      if (!avrprog.writeImage(hexFileName,
                              2048,//for 64B pages, will read 32 at a time, for 512 pages, will be 4 at a time
                              16384)) {//flash size, TODO, make this configurable from SD
        digitalWrite(greenLED_pin, LOW);
        digitalWrite(yellowLED_pin, LOW);
        digitalWrite(redLED_pin, HIGH);//error
        avrprog.error(F("Failed to write flash"));
        while (1) {
        }

      }
      for (int i = 0; i < 10; i++) {
        digitalWrite(greenLED_pin, HIGH);
        digitalWrite(yellowLED_pin, HIGH);
        digitalWrite(redLED_pin, HIGH);
        delay(50);
        digitalWrite(greenLED_pin, LOW);
        digitalWrite(yellowLED_pin, LOW);
        digitalWrite(redLED_pin, LOW);
        delay(50);
      }
      Serial.printf("DONE in %u msec\n", (millis() - startTime));
    } else {
      digitalWrite(greenLED_pin, LOW);
      digitalWrite(yellowLED_pin, LOW);
      digitalWrite(redLED_pin, HIGH);//error
      while (1) {
        //
      }
    }
  }
}
