#ifndef YM_H
#define YM_H

#include <Arduino.h>

class Ym {
    public:
        Ym(int sda, int scl, int expanderId);
        void begin();
        void reset();
        void writeReg(uint8_t reg, uint8_t value);
        void led(boolean on);

    protected:
        
    private:
        int m_expander;
        int m_sda;
        int  m_scl;

        byte xpLedState = 0xff;
        byte xpControlState = 0xff;

        void rwait();
        void xpSend(uint8_t port, uint8_t val);
        int xpRead(uint8_t port);
        void xpInitialize();
        int ymReadStatus();
        void ymWaitBusy();
        void ymControl(byte val);
        void ymData(byte val);
        void ymReset();
};

#endif
