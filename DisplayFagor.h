/*
 Copyright © 2015 José Luis Zabalza  License LGPLv3+: GNU
 LGPL version 3 or later <http://www.gnu.org/copyleft/lgpl.html>.
 This is free software: you are free to change and redistribute it.
 There is NO WARRANTY, to the extent permitted by law.
*/

#include <avr/io.h>
#include <avr/interrupt.h>


class DisplayFagor
{
  public:
  
// We can not allow another object of this class, so this class
// is a singleton.
    static DisplayFagor & instance()
    {
        static DisplayFagor o;
        return o;
    }

    void initialize();

// Functions for control display
    void refresh();

    void setRefreshFreq(unsigned newFreq);

    void setGreenLed(bool status);
    void setRedLed(bool status);
    void setDisplayValue(unsigned val);

protected:
    DisplayFagor();
private:
    bool m_bGreenLedStatus;
    bool m_bRedLedStatus;
    unsigned m_uDisplayValue;


    void setShiftRegisterValue(uint8_t value);

    uint8_t getShiftRegisterValueForDigit(uint8_t value);
    void setPrescaler(unsigned divisor);
    void setAnode(int index);
};

