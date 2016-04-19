#include <EtherCard.h>

// ethernet interface mac address, must be unique on the LAN
//static byte mymac[] = { 0x08,0x2F,0x5E,0x83,0x06,0x48 };
static byte mymac[] = { 0x54,0x65,0x76,0x87,0x98,0x21 };

byte Ethernet::buffer[700];
static uint32_t timer;

const char website[] PROGMEM = "parkingmonitor.ddns.net";
const char* serial = "HC-SR04-1";

int sonicTrig = 3;
int sonicEcho = 2;

#define avgArrayLength 10

float lastTransmittedData = 10000;

long microsecondsToCentimeters(long duration);
long readUltraSonic();
long microsecondsToCentimeters(long duration);
void sendData(float data);


long readUltraSonic() {
  digitalWrite(sonicTrig, LOW);
  delayMicroseconds(2);
  digitalWrite(sonicTrig, HIGH);
  delayMicroseconds(5);
  digitalWrite(sonicTrig, LOW);

  long duration = pulseIn(sonicEcho, HIGH);
  return microsecondsToCentimeters(duration);
}

long microsecondsToCentimeters(long duration) {
  return duration / 29 / 2;
}


// called when the client request is complete
static void my_callback (byte status, word off, word len) {
  Serial.println(">>>");
  Ethernet::buffer[off+300] = 0;
  Serial.print((const char*) Ethernet::buffer + off);
  Serial.println("...");
}

void setup () {
  pinMode(sonicTrig, OUTPUT);
  pinMode(sonicEcho, INPUT);
  
  Serial.begin(57600);
  Serial.println(F("\n[webClient]"));

  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0) 
    Serial.println(F("Failed to access Ethernet controller"));
  if (!ether.dhcpSetup())
    Serial.println(F("DHCP failed"));

  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);  
  ether.printIp("DNS: ", ether.dnsip);  

  if (!ether.dnsLookup(website))
    Serial.println("DNS failed");
    
  ether.printIp("SRV: ", ether.hisip);
}

void sendData(float data) {  
  Serial.println();
  Serial.println("<<< REQ ");
  //String reqData = "serial=";
  //reqData += String(serial);
  //reqData += "&distance=";
  //reqData += String(data);
  
  //char reqCharData[64];
  //strcpy(reqCharData, reqData.c_str());
  //Serial.println(reqData);
  //reqCharData[0] = "serial=HC-SR04-1&distance=";
  //char* dataChar = "       ";
  //int dataInt = (int)data;
  //sprintf(reqCharData, "serial=HC-SR04-1&distance=%d", dataInt);
  //reqCharData[26] = dataChar;
  //const char* reqConstCharData = reqCharData;
  //reqData.toCharArray(reqCharData, 64);
  //Serial.println(reqCharData);
  if (data > 200) {
    ether.browseUrl(PSTR("/add_data.php?"), "serial=HC-SR04-1&distance=400" , website, my_callback);
  } else {
    ether.browseUrl(PSTR("/add_data.php?"), "serial=HC-SR04-1&distance=100" , website, my_callback);
  }
  
  //delete[] reqCharData;
  //delete[] reqConstCharData;
}

void loop () {
  if (timer < millis()) {
    long sum = 0;
    for (int i=0; i<avgArrayLength; ++i) {
      sum += readUltraSonic();
      delay(500);
    }
    float data = (float)sum/avgArrayLength;
    if (lastTransmittedData - data > 10 || data - lastTransmittedData > 10) {
      sendData(data);
      Serial.println(data);
      lastTransmittedData = data;
    }
    timer = millis() + 5000;
  }
  ether.packetLoop(ether.packetReceive());
}
