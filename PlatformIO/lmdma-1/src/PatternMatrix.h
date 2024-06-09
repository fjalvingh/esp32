#ifndef PatternMatrix_H
#define PatternMatrix_H

class PatternMatrix : public Drawable {
  private:
    byte theta = 0;
    byte hueoffset = 0;

  public:
    PatternMatrix() {
      name = (char *)"Matrix";
    }

    unsigned int drawFrame() {
        // effects.DimAll(192); 
        CRGB green = CRGB(175, 255, 175);

        // if(random8(2) == 0) {
            for(int i = 0; i < 4; i++) {
                uint8_t spx = random8(MATRIX_WIDTH);
                effects.leds[XY(spx, 0)] = green;
            }
        // }

        //-- Clear the bottom row
        for(uint8_t x = 0; x < MATRIX_WIDTH; x++) {
            effects.leds[XY(x, MATRIX_HEIGHT - 1)] = CRGB::Black;
        }

        for(uint8_t x = 0; x < MATRIX_WIDTH; x++) {

            for(uint8_t y = MATRIX_HEIGHT-1; y > 0; y--) {
                // Serial.printf_P(PSTR("(x,y)=(%d,%d)\n"), (int)x, (int)y);
                uint16_t pos = XY(x, y - 1);
                CRGB col = effects.leds[pos];

                uint16_t tpos = XY(x, y);
                if(col == green) {                      // Bright color (end of trail)
                    effects.leds[tpos] = col;           // Move 1 row down
                }
                effects.leds[pos].nscale8(192);
            }
        }
        effects.ShowFrame();
      return 0;
    }
};

#endif
