#ifndef MY_CONFIG_H
#define MY_CONFIG_H

#include <LiquidCrystal.h>  // for LCD (LiquidCrystal by Arduino, Adafruit (v.1.0.7))
#include <MFRC522.h>        // for RFID (MFRC522 by GithubCommunity (v.1.4.7))
#include <SPI.h>            // for RFID and RTC
#include <Ethernet.h>       // for ethernet shield (Ethernet by VARIOUS (v.2.0.0))
#include <EthernetUdp.h>    // for RTC
#include <TimeLib.h>        // for RTC (Time by Michael Margolis (v.1.6.0))

//================ Important variables ==============
#define NO_OF_CARDS 3
bool LCDFlag = true;
String devID = "myStringHere";
int id[NO_OF_CARDS][4] = {
  {[Number], [Number], [Number], [Number]},
  {[Number], [Number], [Number], [Number]},
  {[Number], [Number], [Number], [Number]}
};
String userName[NO_OF_CARDS] = {
  "[Name]",
  "[Name]",
  "[Name]"
};

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
#define TIME_ZONE 7                         // Waktu Indonesia Barat
#define NTP_PACKET_SIZE 48                  // NTP time stamp is in the first 48 bytes of the message
const char timeServer[] = "time.nist.gov";  // time.nist.gov NTP server
byte packetBuffer[NTP_PACKET_SIZE];         //buffer to hold incoming and outgoing packets
EthernetUDP Udp;
unsigned int localPort = 8888;              // local port to listen for UDP packets

//RFID
#define CS_RFID 4   //pin 4
#define RST_RFID 9  //pin 9
MFRC522 rfid(CS_RFID, RST_RFID);
MFRC522::MIFARE_Key key;
String IDName;
int poi = 99;   // Person Of Interest (used in verifyUserCheckin)
int tempId[4] = {0, 0, 0, 0};
bool checkInStatus[NO_OF_CARDS] = {false, false, false};

// time information
#define CHECK_IN_HOUR 9
#define CHECK_IN_MINUTE 5
int userCheckInHour;
int userCheckInMinute;

#endif
