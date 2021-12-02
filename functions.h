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

void message(bool checkIn) {
  // if going to work
  if (checkIn)
  {
    if (LCDFlag) {
      lcd.clear();
      lcd.print("Welcome");
      lcd.setCursor(0, 1);
      lcd.print(IDName);
      delay(1500);
    }
    else {
      Serial.println("Welcome");
      Serial.println(IDName);
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
      delay(1500);
    }
    else {
      Serial.println("Goodbye");
      Serial.println(IDName);
    }
  }
}
