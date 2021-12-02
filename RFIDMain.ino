#include "my_config.h"
#include "functions.h"

void setup() {
  // Init Serial port or LCD and User Feedback
  if (LCDFlag) {
    lcd.begin(16, 2);
    lcd.print("Initializing ");
  }
  else {
    Serial.begin(9600);
    Serial.println("Initializing");
  }

  // Init SPI bus
  SPI.begin();

  // Init MFRC522 (RFID)
  rfid.PCD_Init();
  for (int i = 0; i < 6; i++)
    key.keyByte[i] = 0xFF;

  if (Ethernet.begin(mac) == 0)
  {
    if (LCDFlag) {
      lcd.clear();
      lcd.print("Ethernet failed");
    }
    else
      Serial.println("Failed to configure Ethernet using DHCP");
    Ethernet.begin(mac, ip);
  }

  // Initialization done
  if (LCDFlag) {
    lcd.clear();
    lcd.print("Put RFID to Scan");
  }
  else
    Serial.println("Setup done!");
}

void loop() {
  // maintains internet connection from timing out
  Ethernet.maintain();

  //look for new cards
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial())
  {
    uploadStatus = false;
    poi = 99;

    // reads card ID and checks if user is in database
    readRFID();

    /*  sends card check (in/out) log to google sheets

        logCard arg: true  == check in to work
                     false == check out of work
    */
    logCard(verifyUserCheckIn());
    if (poi != 99) {
      if (uploadStatus)
        /*  sends message to serial/lcd

            message arg: true  == check in to work  || true  == is late
                         false == check out of work || false == is NOT late
        */
        message(!verifyUserCheckIn());
    }
    if (LCDFlag) {
      lcd.clear();
      lcd.print("Put RFID to scan");
    }
    else
      Serial.println("Put RFID to scan");
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }
  delay(50);
}
