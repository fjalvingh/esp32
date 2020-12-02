class AllWhite {
  private:
    uint8_t seconds = 0;
  
  public:
  AllWhite(){
  };

  void setup() {
    FastLED.clear();
  }

  boolean loop() {

    EVERY_N_SECONDS(1) {
      seconds++;
    }

    // Rainbow background
    if(seconds % 10 == 0) {
      
      for( byte y = 0; y < leds.Height(); y++) {
        for( byte x = 0; x < leds.Width(); x++) {
          leds(x, y) = CRGB::White;
        }
      }
      FastLED.show();
      delay(5000);
    }
  
    return true;
  }
 
};
