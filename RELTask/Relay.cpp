#include "Relay.h"

void Relay::setup(uint8_t p, rel_t tp, bool lvl)
{
    pin = p;
    state.level = lvl;

    pinMode(pin, OUTPUT);

    state.type = tp;
    state.armed = false;
}

void Relay::setState(bool s)
{
    state.ison = s;
    syncro();
}

bool Relay::isOn()
{
    return state.ison;
}

void Relay::setOn()
{

    setState(true);
}

void Relay::setOff()
{

    setState(false);
}

void Relay::syncro()
{
    if (state.ison){
        digitalWrite(pin, state.level?HIGH:LOW);
    }else{
        digitalWrite(pin, state.level?LOW:HIGH);
    }
    
}

bool Relay::swc()
{
    if (state.ison)
        setState(false);
    else
        setState(true);
    return state.ison;
}

void Relay::arm()
{
    if (state.type == RELTYPE_BUTTON)
    {
        state.armed = true;
        setOn();
    }
}

bool Relay::isButton()
{
    return state.type == RELTYPE_BUTTON;
}

void Relay::disarm()
{
    if (state.type == RELTYPE_BUTTON)
    {
        state.armed = false;
        setOff();
    }
}