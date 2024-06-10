#ifndef YM_H
#define YM_H

#include <Arduino.h>

enum LfoWaveform {
    Sawtooth, Square, Triangle, Noise
};

class Channel;

class Operator;

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

        /*----- Per channel data ------*/

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

        inline Channel& channel(int index) {
            if(index > 8 || index < 0)
                index = 0;
            return *m_channels[index];
        }

        /*----- Per channel + operator data ------*/
        /**
         * Set an operator to either enabled or disabled. This takes
         * effect with the next keyon/keyoff command. The operator
         * is in the normal operator order.
         */
        void operatorOn(uint8_t channel, uint8_t op, boolean on);

        /**
         * Set total volume attenuation (0=max, 0x7f=min), regs 0x60++
         */
        void setOpVolume(uint8_t channel, uint8_t op, uint8_t value);

        /**
         * Set the frequency multiplier (0..f, reg 0x40++).
         */
        void setFrequencyMultiplier(uint8_t channel, uint8_t op, uint8_t value);

        /**
         * Set detune (0..7, reg 0x40++)
         */
        void setDetuneFine(uint8_t channel, uint8_t op, uint8_t value);
        
        /**
         * Set detune (0..3, reg 0xC0++)
         */
        void setDetuneCoarse(uint8_t channel, uint8_t op, uint8_t value);

        /**
         * Set both fine and coarse detune in one go, 0..31.
         */
        void setDetune(uint8_t channel, uint8_t op, uint8_t value);

        /**
         * Set the attack rate (0..31, reg 0x80..)
         */
        void setAttackRate(uint8_t channel, uint8_t op, uint8_t value);

        /**
         * Set Key Scaling (0..3) (ADSR rate scaling, KS field of 0x80++)
         */
        void setAdsrRateScaling(uint8_t channel, uint8_t op, uint8_t value);

        /**
         * Decay rate 0..31
         */
        void setDecayRate(uint8_t channel, uint8_t op, uint8_t value);

        /**
         * Decay level 0..15
         */
        void setDecayLevel(uint8_t channel, uint8_t op, uint8_t value);

        /**
         * Enable AM Modulation.
         */
        void setAmEnable(uint8_t channel, uint8_t op, boolean on);

        /**
         * Set decay rate 2 (0..31), during the sustain phase.
         */
        void setSustainDelayRate(uint8_t channel, uint8_t op, uint8_t value);

        /**
         * Set the release rate (0..15)
         */
        void setReleaseRate(uint8_t channel, uint8_t op, uint8_t value);


        /*------ Loading patches -------*/
        void loadPatch(uint8_t* patch);

        void loadPatchGlobal(uint8_t* patch);

        void loadPatch(uint8_t channel, uint8_t* patch);


    protected:
        
    private:
        int m_expander;
        int m_sda;
        int  m_scl;

        byte xpLedState = 0xff;
        byte xpControlState = 0xff;

        /** Whether a specific operator is ON or OFF per channel */ 
        byte m_operatorOnOffState[8];     

        /** The last written value of every register */
        byte m_regState[256];

        Channel* m_channels[8];
        Operator* m_operators[8 * 4];

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

class Channel {
    friend class Ym;

    private:
        Ym* m_ym;
        int m_channel;

    protected:
        Channel(Ym* ym, int channel);

    public:
        inline Channel& note(boolean on) {
            m_ym->note(m_channel, on);
            return *this;
        }

        inline Channel& noteOn() {
            m_ym->noteOn(m_channel);
            return *this;
        }

        inline Channel& noteOff() {
            m_ym->noteOff(m_channel);
            return *this;
        }

        inline Channel& algorithm(uint8_t algo) {
            m_ym->setAlgorithm(m_channel, algo);
            return *this;
        }

        inline Channel& tone(uint8_t keyCode, uint8_t keyFraction) {
            m_ym->setTone(m_channel, keyCode, keyFraction);
            return *this;
        }

        /**
         * Set M1 feedback level (0..7, reg 0x20).
         */
        inline Channel& feedback(uint8_t level) {
            m_ym->setFeedback(m_channel, level);
            return *this;
        }

        /**
         * Set the output channels (left, right) on or off, bit 1 = right, bit 0 = left.
         */
        inline Channel& outputChannels(uint8_t lr) {
            m_ym->setOutputChannels(m_channel, lr);
            return *this;
        }

        /**
         * Set sensitivity 0..7 (reg 0x38..0x3f)
         */
        inline Channel& phaseModulationSensitivity(uint8_t level) {
            m_ym->setPhaseModulationSensitivity(m_channel, level);
            return *this;
        }

        /**
         * Set AM sensitivity 0..3 (reg 0x38..0x3f)
         */
        inline Channel& amplitudeModulationSensitivity(uint8_t level) {
            m_ym->setAmplitudeModulationSensitivity(m_channel, level);
            return *this;
        }
};

class Operator {
    friend Ym;

    private:
        Ym* m_ym;
        uint8_t m_channel;
        uint8_t m_op;

    protected:
        Operator(Ym* ym, uint8_t channel, uint8_t op);



};

#endif
