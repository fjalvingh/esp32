#ifndef YM_H
#define YM_H

#include <Arduino.h>

enum LfoWaveform {
    Sawtooth, Square, Triangle, Noise
};

class Ym {
    public:
        Ym(int sda, int scl, int expanderId);
        void begin();
        void reset();
        void writeReg(uint8_t reg, uint8_t value);

        /**
         * Write a new value in a register, but only change the bits
         * that are 1 in mask. This uses the saved value for each
         * register to detect the other bits.
         */
        void writeRegM(uint8_t reg, uint8_t value, uint8_t mask);

        void led(boolean on);

    /** Global data */
        void lfo(boolean on);

        /**
         * Set LFO freq, 0 = 0.008Hz, 0xff = 32.6Hz.
         */
        void setLfoFreq(uint8_t freq);

        void setLfoWaveForm(uint8_t code);

        void setLfoWaveForm(LfoWaveform form);

        /**
         * Phase modulation (vibrato) depth.
         */
        void setLfoPhaseDepth(uint8_t depth);

        /**
         * Amplitude modulation depth.
         */
        void setLfoAmplitudeDepth(uint8_t depth);

        /**
         * When enabled (on), C2 of channel 7 will use a noise
         * waveform instead of a sine waveform.
         */
        void setNoise(boolean on, uint8_t freq);


        /**
         * Set an operator to either enabled or disabled. This takes
         * effect with the next keyon/keyoff command. The operator
         * is in the normal operator order.
         */
        void operatorOn(uint8_t channel, uint8_t op, boolean on);

        /**
         * Start a note on a channel. Before use this needs to set the
         * operator(s) to use for the channel (at least once) with
         * operatorOn(). If not only the C2 operator gets started.
         */
        void noteOn(uint8_t channel);

        /**
         * Switch a note off on a channel. Like noteOn, this switches
         * off the operators set with operatorOn().
         */
        void noteOff(uint8_t channel);

        /**
         * Either start or stop a note.
         */
        void note(uint8_t channel, boolean on);

        void setTone(uint8_t channel, uint8_t keyCode, uint8_t keyFraction);

        /**
         * Set the algorith (0..7) (reg 20).
         */
        void setAlgorithm(uint8_t channel, uint8_t algo);

        /**
         * Set M1 feedback level (0..7, reg 0x20).
         */
        void setFeedback(uint8_t channel, uint8_t level);

        /**
         * Set the output channels (left, right) on or off, bit 1 = right, bit 0 = left.
         */
        void setOutputChannels(uint8_t channel, uint8_t lr);

        /**
         * Set sensitivity 0..7 (reg 0x38..0x3f)
         */
        void setPhaseModulationSensitivity(uint8_t channel, uint8_t level);

        /**
         * Set AM sensitivity 0..3 (reg 0x38..0x3f)
         */
        void setAmplitudeModulationSensitivity(uint8_t channel, uint8_t level);

    protected:
        
    private:
        int m_expander;
        int m_sda;
        int  m_scl;

        byte xpLedState = 0xff;
        byte xpControlState = 0xff;

        /** Whether a specific operator is ON or OFF per channel, used to set reg.8 */
        byte m_operatorOnOffState[8];

        /** The last written value of every register */
        byte m_regState[256];

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
