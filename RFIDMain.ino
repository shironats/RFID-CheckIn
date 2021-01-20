#include <LiquidCrystal.h>  // for LCD (LiquidCrystal by Arduino, Adafruit (v.1.0.7))
#include <MFRC522.h>        // for RFID (MFRC522 by GithubCommunity (v.1.4.7))
#include <SPI.h>            // for RFID and RTC
#include <TimeLib.h>        // for RTC (Time by Michael Margolis (v.1.6.0))
#include <EthernetUdp.h>    // for RTC
#include <Ethernet.h>       // for ethernet shield (Ethernet by VARIOUS (v.2.0.0))

//================ Important variables ==============
#define No_of_Cards 3
bool LCDFlag = true;
int id[No_of_Cards][4] = {
  {[idHere], [idHere], [idHere], [idHere]},
  {[idHere], [idHere], [idHere], [idHere]},
  {[idHere], [idHere], [idHere], [idHere]}
};
String userName[No_of_Cards] = {
  "[idHere]",
  "[idHere]",
  "[idHere]"
};
String devID = "[idHere]";
//================ Important variables ==============

//LCD
LiquidCrystal lcd(3, 2, A0, A1, A2, A3);

//Ethernet
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 0, 108);
EthernetClient client;
char server[] = "api.pushingbox.com";
bool uploadStatus = false;

// Online RTC
IPAddress timeServer(203, 160, 128, 3);   // google "ntp server ip address indonesia"
const int timeZone = 7;                   // Waktu Indonesia Barat
EthernetUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

//RFID
#define CS_RFID 4   //pin 4
#define RST_RFID 9  //pin 9
MFRC522 rfid(CS_RFID, RST_RFID);
MFRC522::MIFARE_Key key;
String IDName;
int poi = 99;   // Person Of Interest (used in verifyUserCheckin)
int id_temp[1][4];
bool checkInStatus[No_of_Cards] = {false, false, false};

// time information
#define checkInHour 9
#define checkInMinute 5
int userCheckInHour;
int userCheckInMinute;

void setup() {

  // Init Serial port or LCD
  if (LCDFlag)
    lcd.begin(16, 2);
  else
    Serial.begin(9600);

  // User feedback
  if (LCDFlag)
    lcd.print("Initializing ");
  else
    Serial.println("Initializing");

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
      //      lcd.setCursor(0, 1);
      //      lcd.print("Please try again");
    }
    else
      Serial.println("Failed to configure Ethernet using DHCP");
    Ethernet.begin(mac, ip);
  }

  // setup connection to receive time packets
  Udp.begin(localPort);
  setSyncProvider(getNtpTime);

  // Checks if time is set
  if (timeStatus() != timeNotSet) {
    if (LCDFlag) {
      lcd.clear();
      lcd.print("RTC connected");
    }
    else
      Serial.println("RTC connected");
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
  if (rfid.PICC_IsNewCardPresent())
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

    if (uploadStatus)
      /*  sends message to serial/lcd

          message arg: true  == check in to work  || true  == is late
                       false == check out of work || false == is NOT late
      */
      message(verifyUserCheckIn(), verifyLate());
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }
  delay(50);
}

void readRFID() {
  if (LCDFlag) {
    lcd.clear();
    lcd.print("Reading card");
  }
  else
    Serial.println("Reading card");

  // read card ID
  rfid.PICC_ReadCardSerial();

  // saves card ID to array for use in verifyUserCheckIn()
  for (int i = 0; i < 4; i++)
    id_temp[0][i] = rfid.uid.uidByte[i];

  delay(500);
}

bool verifyUserCheckIn() {
  // checks every card for matching ID
  for (int i = 0; i < No_of_Cards; i++)
  {
    // checks each part of the card ID against database
    if (id[i][0] == id_temp[0][0])
    {
      if (id[i][1] == id_temp[0][1])
      {
        if (id[i][2] == id_temp[0][2])
        {
          if (id[i][3] == id_temp[0][3])
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
      }
    }
  }
  return false;
}


void logCard(bool checkInOut) {
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

    checkInStatus[poi] = !checkInOut;
    uploadStatus = true;
    delay(1000);
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
  if ((userCheckInHour < checkInHour) || ((userCheckInHour == checkInHour) && (userCheckInMinute <= checkInMinute)))
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
  // if going home or card is not in database
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
const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 5000) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
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
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
