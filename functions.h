void readRFID() {
  if (LCDFlag) {
    lcd.clear();
    lcd.print("Reading card");
  }
  else
    Serial.println("Reading card");

  // saves card ID to array for use in verifyUserCheckIn()
  for (int i = 0; i < 4; i++)
    tempId[i] = rfid.uid.uidByte[i];

  delay(500);
}

bool verifyUserCheckIn() {
  // checks every card for matching ID
  for (int i = 0; i < NO_OF_CARDS; i++)
  {
    // checks each part of the card ID against database
    if ((id[i][0] == tempId[0]) && (id[i][1] == tempId[1]) && (id[i][2] == tempId[2]) && (id[i][3] == tempId[3]))
    {
      IDName = userName[i];
      // Person Of Interest
      poi = i;

      // Save check in time;
      userCheckInHour = hour();
      userCheckInMinute = minute();

      return !checkInStatus[i];
    }
  }
  return false;
}

void logCard(bool checkInOut) {
  // immediately exit if ID is not recognized
  if (poi == 99) return;
  // connects to internet
  if (client.connect(server, 80)) {

    // concantenate into a string for sending request to google sheets
    client.print("GET /pushingbox?devid=");
    client.print(devID);
    client.print("&Lembar=");
    if (checkInOut) // checking in
      client.print("1");
    else            // checking out
      client.print("2");
    client.print("&IDNum=");
    for (int i = 0; i < 4; i++)
      client.print(rfid.uid.uidByte[i]);
    client.print(" ");
    client.print("HTTP/1.1");
    client.println();
    client.println("Host: api.pushingbox.com");
    client.println("Connection: close");
    client.println();

    // display time and upload confirmation
    if (LCDFlag) {
      lcd.clear();
      lcd.print(day());
      lcd.print('/');
      lcd.print(month());
      lcd.print('/');
      lcd.print(year());
      
      lcd.setCursor(10, 0);
      if (hour() < 10)
        lcd.print('0');
      lcd.print(hour());
      lcd.print(':');
      if (minute() < 10)
        lcd.print('0');
      lcd.print(minute());
      lcd.setCursor(0, 1);
      lcd.print("Uploaded...");
    }
    else
      Serial.println("Uploaded...");

    // change user's check in status
    checkInStatus[poi] = checkInOut;
    uploadStatus = true;
    delay(500);
  }
  // failed to create connection
  else {
    if (LCDFlag) {
      lcd.clear();
      lcd.print("Upload failed");
      lcd.setCursor(0, 1);
      lcd.print("Please try again");
      delay(1000);
      lcd.clear();
      lcd.print("Put RFID to Scan");
    }
    else
      Serial.println("Connection failed\nTry again\nPut RFID to scan");
  }
}

bool verifyLate() {
  // if on time
  if ((userCheckInHour < CHECK_IN_HOUR) || ((userCheckInHour == CHECK_IN_HOUR) && (userCheckInMinute <= CHECK_IN_MINUTE)))
    return false;
  // if late
  else
    return true;
}

void message(bool checkIn, bool isLate) {
  // if going to work
  if (checkIn)
  {
    // if not late
    if (!isLate)
    {

      if (LCDFlag) {
        lcd.clear();
        lcd.print("Welcome");
        lcd.setCursor(0, 1);
        lcd.print(IDName);
        delay(3000);
        lcd.clear();
        lcd.print("Put RFID to scan");
      }
      else {
        Serial.println("Welcome");
        Serial.println(IDName);
        Serial.println("Put RFID to scan");
      }
    }
    // if late
    else
    {
      if (LCDFlag) {
        lcd.clear();
        lcd.print("You are late");
        lcd.setCursor(0, 1);
        lcd.print(IDName);
        delay(3000);
        lcd.clear();
        lcd.print("Put RFID to scan");
      }
      else {
        Serial.println("You are late");
        Serial.println(IDName);
        Serial.println("Put RFID to scan");
      }
    }
  }
  // if going home
  else
  {
    if (LCDFlag) {
      lcd.clear();
      lcd.print("Goodbye");
      lcd.setCursor(0, 1);
      lcd.print(IDName);
      delay(3000);
      lcd.clear();
      lcd.print("Put RFID to scan");
    }
    else {
      Serial.println("Goodbye");
      Serial.println(IDName);
      Serial.println("Put RFID to scan");
    }
  }
}

/*-------- NTP code ----------*/
// send an NTP request to the time server at the given address
void sendNTPpacket(const char * address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); // NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
