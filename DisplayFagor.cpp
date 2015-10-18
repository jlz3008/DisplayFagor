/*
 * Copyright © 2015 José Luis Zabalza  License LGPLv3+: GNU
 * LGPL version 3 or later <http://www.gnu.org/copyleft/lgpl.html>.
 * This is free software: you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */

#include <Arduino.h>
#include <avr/io.h>
#include <util/delay.h>
#include "DisplayFagor.h"

//-------[Variables used on ISR]----------

static volatile bool sbItsTimeForRefresh;

#define DISPLAY_FAGOR_PRESCALER     256

#define PINPORT_ANODE_1  0
#define PINPORT_ANODE_2  1
#define PINPORT_ANODE_3  2
#define PINPORT_CLK_SR   3
#define PINPORT_STORE_SR 4
#define PINPORT_DATA_SR  5

//--------------------------------------------------------------------------
ISR(TIMER5_COMPA_vect)
{
    sbItsTimeForRefresh = true;
}

//--------------------------------------------------------------------------
DisplayFagor::DisplayFagor() :
    m_bGreenLedStatus(true), // Off
    m_bRedLedStatus(true)
{
    // Some library or code initialize Timer/Counter 5 so we can't do
    // something here
}
//--------------------------------------------------------------------------
void DisplayFagor::initialize()
{
    uint8_t sreg;
// disconnect interrupt to atomic operation to no disturb
    sreg = SREG;
    cli();

// initialize sequence
    TCCR5A = 0;                        // OC5A,OC5B and OC5C as normal
                                       // GPIO function and Timer Mode
                                       // = CTC (WGM51 = WGM50 = 0)
    TCCR5B = 0x08;                     // Noise Canceller = off, Edge
                                       // Input capture = falling
                                       // Timer Mode = CTC (WGM53 = 0
                                       // WGM52 = 1) Prescaler = Clock
                                       // Stopped
    TCCR5C = 0;                        // Force Output compare
                                       // (A,B,C)= none
    TCNT5 = 0;                         // Timer counter = 0
    TIMSK5 = _BV(OCIE5A);              // Output Compare interrupt

    setRefreshFreq(25);

    DDRF |= 0x3F;                      // Init Port F 0..5 to output.
                                       // (Pins 0..5 from Analog IN on
                                       // Arduino MEGA )
    PORTF &= 0x3F;

    setPrescaler(DISPLAY_FAGOR_PRESCALER);  // prescaler ON The count
                                            // begins

    // this set interrupt in the last status. I'm supposed to be
    // activated at some point
    SREG = sreg;

}
//--------------------------------------------------------------------------
void DisplayFagor::setRefreshFreq(unsigned newFreq)
{
    OCR5A = (16000000/2/DISPLAY_FAGOR_PRESCALER)/newFreq - 1;
}
//--------------------------------------------------------------------------
void DisplayFagor::refresh()
{
    unsigned val = m_uDisplayValue;

    sbItsTimeForRefresh = false;

    setShiftRegisterValue(getShiftRegisterValueForDigit(val % 10) | ((m_bRedLedStatus) ? 0x80 : 0x00));
    setAnode(3);
    _delay_loop_1(0);
    val /= 10;
    setAnode(0);
    setShiftRegisterValue(getShiftRegisterValueForDigit(val % 10) | ((m_bGreenLedStatus) ? 0x80 : 0x00));
    val /= 10;
    setAnode(2);
    _delay_loop_1(0);
    setAnode(0);
    setShiftRegisterValue(getShiftRegisterValueForDigit(val % 10) | ((m_bGreenLedStatus) ? 0x80 : 0x00));
    val /= 10;
    setAnode(1);
    _delay_loop_1(0);
    setAnode(0);

}
//--------------------------------------------------------------------------
void DisplayFagor::setShiftRegisterValue(uint8_t value)
{
    int i;
    for(i = 0; i < 8; i++)
    {
        if(value & 1)
            PORTF |= (1<<PINPORT_DATA_SR);
        else
            PORTF &= ~(1<<PINPORT_DATA_SR);


        PORTF |= (1<<PINPORT_CLK_SR);
        _delay_loop_1(15);    // Tw min = 24 nS See datasheet 74HC595.
                              // On Arduino MEGA 16MHz Clock -> CPU
                              // cicle = 0.68 nS
                              // 24 nS / 0.68 (ns/Cicle) = 36 cicles.
                              // from _delay_loop_1() documentantion
                              //       Delay loop using an 8-bit
                              // counter, so up to 256
                              //       iterations are possible. (The
                              // value 256 would have to be passed
                              //       as 0.) The loop executes three
                              // CPU cycles per iteration, not
                              //       including the overhead the
                              // compiler needs to setup the counter
                              //       register.
                              //  36 cicles/3 (cicles/loop) = 12 loops
                              //  so we make 15 cicles (just in case)

        PORTF &= ~(1<<PINPORT_CLK_SR);
        _delay_loop_1(15);

        value >>= 1;
    }

// Store

    PORTF |= (1<<PINPORT_STORE_SR);
    _delay_loop_1(15);    // Tw min = 24 nS See datasheet 74HC595. On
                          // Arduino MEGA 16MHz Clock -> CPU cicle =
                          // 0.68 nS
    PORTF &= ~(1<<PINPORT_STORE_SR);
    _delay_loop_1(15);
}

//--------------------------------------------------------------------------
void DisplayFagor::setAnode(int index)
{
    switch(index)
    {
        case 1: PORTF = ((PORTF & ~(_BV(PINPORT_ANODE_1) | _BV(PINPORT_ANODE_2) | _BV(PINPORT_ANODE_3))) | _BV(PINPORT_ANODE_1)); break;
        case 2: PORTF = ((PORTF & ~(_BV(PINPORT_ANODE_1) | _BV(PINPORT_ANODE_2) | _BV(PINPORT_ANODE_3))) | _BV(PINPORT_ANODE_2)); break;
        case 3: PORTF = ((PORTF & ~(_BV(PINPORT_ANODE_1) | _BV(PINPORT_ANODE_2) | _BV(PINPORT_ANODE_3))) | _BV(PINPORT_ANODE_3)); break;

        case 0:
        default:
            PORTF = ((PORTF & ~(_BV(PINPORT_ANODE_1) | _BV(PINPORT_ANODE_2) | _BV(PINPORT_ANODE_3)))); break;
    }
}

//--------------------------------------------------------------------------
uint8_t DisplayFagor::getShiftRegisterValueForDigit(uint8_t value)
{
    uint8_t Result = 0;
    switch(value)
    {
        case 0: Result = 0x01; break;
        case 1: Result = 0x79; break;
        case 2: Result = 0x12; break;
        case 3: Result = 0x30; break;
        case 4: Result = 0x68; break;
        case 5: Result = 0x24; break;
        case 6: Result = 0x04; break;
        case 7: Result = 0x71; break;
        case 8: Result = 0x00; break;
        case 9: Result = 0x20; break;
        default: Result = 0x7F; break;
    }
    return(Result);
}

//--------------------------------------------------------------------------
void DisplayFagor::setGreenLed(bool status)
{
    m_bGreenLedStatus = !status;
}

//--------------------------------------------------------------------------
void DisplayFagor::setRedLed(bool status)
{
    m_bRedLedStatus = !status;
}

//--------------------------------------------------------------------------
void DisplayFagor::setDisplayValue(unsigned val)
{
    m_uDisplayValue = val;
}

//--------------------------------------------------------------------------
void DisplayFagor::setPrescaler(unsigned divisor)
{
    uint8_t prescaler;
    switch(divisor)
    {
        case    1: prescaler = 1; break;
        case    8: prescaler = 2; break;
        case   64: prescaler = 3; break;
        case  256: prescaler = 4; break;
        case 1024: prescaler = 5; break;
        default:
        case    0: prescaler = 0; break;
    }

    if(prescaler <= 5)
        TCCR5B = (TCCR5B & ~(_BV(CS52) | _BV(CS51) | _BV(CS50))) | prescaler;
}
