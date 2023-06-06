#ifndef _RELAYS_h
#define _RELAYS_h
#include <Arduino.h>
#include <GlobalSettings.h>


class Relay{
    public:
    //Relay();
    void setup(uint8_t p, rel_t tp=RELTYPE_SWICH, bool lvl=false);
    bool isOn();
    void setOn();
    void setOff();
    bool swc();
    void arm();
    void disarm();
    bool isButton();
    void setState(boolean s);
    protected :
    void syncro();
    //rel_t type;
    relState_t state;
    //bool ison;
    uint8_t pin;
    //bool level;
    //bool armed;
};


#endif