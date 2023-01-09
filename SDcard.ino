
//HSPI MOSI=13, MISO=12, CLK=14, CS=25
//VSPI MOSI=23, MISO=19, CLK=18, CS=25

void getFileNames() {
  Serial.println("Looking for files");
  File root;
  root = SD.open("/");
  while (true) {

    File entry =  root.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    char *pointerToHexKey = strstr(entry.name(), hexKey);//go find keyword
    if (pointerToHexKey != NULL) {
      char *pointerToHiddenFile = strstr(entry.name(), hiddenFileKey);
      if (pointerToHiddenFile == NULL) {//good not hidden file
        Serial.print("Found Hex File: ");
        strcpy(hexFileName, "/");
        strcat(hexFileName, entry.name());
        Serial.println(hexFileName);
      }
    }
 

    Serial.println(entry.name());
    //    if (entry.isDirectory()) {
    //      Serial.println(" / ");
    //      //printDirectory(entry, numTabs + 1);
    //    } else {
    //      // files have sizes, directories do not
    //      Serial.print("\t\t");
    //      Serial.println(entry.size(), DEC);
    //    }
    entry.close();
  }
}


void initSDcard() {
  //spiSD.begin(14, 12, 13, SD_CS); //SCK,MISO,MOSI,SS //HSPI1
  spiSD.begin(18, 19, 23, SD_CS_pin); //SCK,MISO,MOSI,SS //HSPI1
  while (!SD.begin( SD_CS_pin, spiSD ))
  {
    Serial.println("Card Mount Failed");
    delay(1000);
  }


  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }
  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: % lluMB\n", cardSize);

  getFileNames();


}
