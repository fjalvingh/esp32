#include <Adafruit_NeoPixel.h>

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Adafruit_NeoPixel.h>

// NEOPIXEL SETUP
#define PIN            14
#define NUMPIXELS      8
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
uint8_t txValue = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++){
          Serial.print(rxValue[i]);
        }
        Serial.println();

        String command = rxValue.c_str();
        command = command.substring(0,2);
    
        if(command == "!C"){
          uint8_t r = (uint8_t)rxValue[2];
          uint8_t g = (uint8_t)rxValue[3];
          uint8_t b = (uint8_t)rxValue[4];

          Serial.printf("R: %d | G: %d | B: %d \n\r",r,g,b);

          for(int i=0;i<NUMPIXELS;i++){
            pixels.setPixelColor(i, pixels.Color(r,g,b));
          }
          pixels.show();
        }


        Serial.println(command);
        Serial.println("*********");
      }
    }
};


void setup() {
  Serial.begin(115200);

  // SETUP NeoPixel
  pixels.begin();

  // Create the BLE Device
  BLEDevice::init("NeoPixel Controller BLE");

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  pCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {

  if (deviceConnected) {
    //Serial.printf("*** Sent Value: %d ***\n", txValue);
    pCharacteristic->setValue(&txValue, 1);
    pCharacteristic->notify();
  }
  delay(1000);
}
