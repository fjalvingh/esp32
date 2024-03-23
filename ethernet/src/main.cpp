#include <SPI.h>
#include <EtherCard.h>
#include "Adafruit_NeoPixel.h"

/**
 * Plan: this should communicate with a central server to get texts to show. The
 * core idea is this:
 * 1. Obtain an IP address and DNS server from DHCP. If this fails-> loop till success.
 * 2. Try to connect to http://etc.to/puzzler/ledio, and pass the parameters mac, rndid to make myself known.
 * 3. The ledio url will return with a command block containing a text.
 * 4. Run the text. Redo the ledio call every 30 seconds to try for a new command block.
 */
const int cBoards = 1;
const int PIN = 13;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(64 * cBoards, PIN, NEO_GRB + NEO_KHZ800);

byte mac[] = {0xde, 0xad, 0xbe, 0xef, 0xa8, 0x0b};
byte ip[] = {192, 168, 1, 123};

byte Ethernet::buffer[500];

static byte myip[] = { 192,168,1,200 };
static byte gwip[] = { 192,168,1,1 };
static byte dnsip[] = {192, 168, 1, 238};
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
static char* m_hispip;

const char dnsName[] PROGMEM = "www.etc.to";

const char page[] PROGMEM =
"HTTP/1.0 503 Service Unavailable\r\n"
"Content-Type: text/html\r\n"
"Retry-After: 600\r\n"
"\r\n"
"<html>"
  "<head><title>"
    "Hello World!"
  "</title></head>"
  "<body>"
    "<h3>Hello World! This is your Arduino speaking!</h3>"
  "</body>"
"</html>";

enum Phase {
  ETHER,              // Finding ethernet controller
  DHCP,               // Get DHCP address
  DNS,                // get host address
  CONN,               // connect to remote & get response
  LOOP,               // display and effects
  ERROR               // FAILED - term
};

Phase m_phase = ETHER;

uint32_t phaseColor(Phase ph) {
  uint32_t col;
  switch(ph) {
    case ETHER: col = 0xff0000l; break;
    case DHCP: col = 0x400000l; break;
    case DNS: col = 0x000040l; break;
    case CONN: col = 0x404000l; break;
    default: col = 0x707070l; break;
  }
  return col;
}

void phaseDisplay(Phase ph) {
  uint32_t col = phaseColor(ph);
  for(int i = 0; i < strip.getPixels(); i++)
    strip.setPixelColor(i, col);
  strip.show();
  Serial.print("Phase ");
  Serial.print(ph);
  Serial.println("");
}

void fatal(Phase ph) {
  // for(int i = 0; i < 5; i++) {
  //   phaseDisplay(ph);
  //   delay(250);
  //   strip.clear();
  //   strip.show();
  //   delay(500);
  // }
}

void setup() {
  m_phase = ETHER;
  strip.begin();
  strip.show();
  Serial.begin(57600);
  Serial.println("\n[Hello World]");

//   if (ether.begin(sizeof Ethernet::buffer, mymac) == 0)
//     Serial.println( "Failed to access Ethernet controller");
//   else {
//     ether.dhcpSetup();
// //    ether.staticSetup(myip, gwip);
//
//     ether.printIp("IP:  ", ether.myip);
//     ether.printIp("GW:  ", ether.gwip);
//     ether.printIp("DNS: ", ether.dnsip);
//   }
}

void runEther() {
  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0) {
    Serial.println( "Failed to access Ethernet controller");
    fatal(ETHER);
  }
  m_phase = DHCP;
}

void runDhcp() {
  // ether.staticSetup(myip, gwip, dnsip);

  ether.dhcpSetup();
  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);
  ether.printIp("DNS: ", ether.dnsip);
  m_phase = DNS;

  while(ether.clientWaitingGw())
    ether.packetLoop(ether.packetReceive());
  Serial.print("Packet loop done");
}

static byte iptest[] = {192,168,1,238};

void runDNS() {
  if(! ether.dnsLookup(dnsName, false)) {
    Serial.println("Failed to get www.etc.to");
    fatal(m_phase);
    return;
  }
  m_hispip = ether.hisip;
  m_phase = CONN;
  m_hispip = iptest;
  ether.printIp("etc.to: ", m_hispip);
  Serial.print("\n");
}

static char* UDP_REQ = "GET /\r\n";


long lastTransmit;

void runConn() {
  int len = ether.packetReceive();
  if(len > 0)
    ether.packetLoop(len);
  long t = millis();
  if(t - lastTransmit < 500)
    return;
  lastTransmit = t;
  ether.sendUdp("helloWorld", 10, 60987, iptest, 8901);
}


/**
 * Connect to remote and ask for a text to show.
 */
void runConnOld() {
  byte* pb = ether.buffer;
  byte* p = pb;

  ether.udpPrepare(65432, m_hispip, 8901);
  memset(pb + UDP_DATA_P, 0, 12);
  memcpy(pb + UDP_DATA_P, mymac, 6);
  ether.udpTransmit(12);
  delay(250);
  // ether.sendUdp(UDP_REQ, sizeof(UDP_REQ), 8901, m_hispip, 8901);
  // delay(1000);
}

void loop() {

  phaseDisplay(m_phase);
  switch(m_phase) {
    case ETHER:
      runEther();
      break;

    case DHCP:
      runDhcp();
      break;

    case DNS:
      runDNS();
      break;

    case CONN:
      runConn();
      break;


  }


  // wait for an incoming TCP packet, but ignore its contents
  // if (ether.packetLoop(ether.packetReceive())) {
  //   memcpy_P(ether.tcpOffset(), page, sizeof page);
  //   ether.httpServerReply(sizeof page - 1);
  // }
}
