#include <UIPEthernet.h>

EthernetUDP udp;
unsigned long next;

void setup() {

  Serial.begin(57600);

  uint8_t mac[6] = {0x00,0x01,0x02,0x03,0x04,0x05};

  // Ethernet.begin(mac,IPAddress(192,168,1,199));
  Ethernet.begin(mac);

  next = millis()+5000;
}

void loop() {

  int success;
  int len = 0;

  if (((signed long)(millis()-next))>0)
    {
      do
        {
          success = udp.beginPacket(IPAddress(192,168,1,238),8901);
          // success = udp.beginPacket(IPAddress(145,131,8,118),8901);
          Serial.print("beginPacket: ");
          Serial.println(success ? "success" : "failed");
          //beginPacket fails if remote ethaddr is unknown. In this case an
          //arp-request is send out first and beginPacket succeeds as soon
          //the arp-response is received.
        }
      while (!success && ((signed long)(millis()-next))<0);
      if (!success )
        goto stop;

      success = udp.write("hello world from arduino");

      Serial.print("bytes written: ");
      Serial.println(success);

      success = udp.endPacket();

      Serial.print("endPacket: ");
      Serial.println(success ? "success" : "failed");

      do
        {
          //check for new udp-packet:
          success = udp.parsePacket();
        }
      while (!success && ((signed long)(millis()-next))<0);
      if (!success )
        goto stop;

      Serial.print("received: '");
      do
        {
          int c = udp.read();
          Serial.write(c);
          len++;
        }
      while ((success = udp.available())>0);
      Serial.print("', ");
      Serial.print(len);
      Serial.println(" bytes");

      //finish reading this packet:
      udp.flush();

      stop:
      udp.stop();
      next = millis()+5000;
    }
}
