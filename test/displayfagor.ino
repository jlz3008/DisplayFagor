/*
 * Copyright © 2015 José Luis Zabalza  License LGPLv3+: GNU
 * LGPL version 3 or later <http://www.gnu.org/copyleft/lgpl.html>.
 * This is free software: you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */


#define VERSION "1.0"

#include <inttypes.h>
#include <assert.h>
#include "DisplayFagor.h"
#include "ansi.h"


static unsigned suDisplayValue;
static bool sbGreenLed;
static bool sbRedLed;
static bool sNeedRefreshData;
static unsigned suRefreshFreq = 25;
//------Local
// prototipes---------------------------------------------------------

static DisplayFagor & DisplayF = DisplayFagor::instance();

//------Local
// prototipes---------------------------------------------------------

void Menu();
void Header();
void IdleProc();

//--------------------------------------------------------------------------------
void setup()
{
    Serial.begin(115200);
    Serial.print("Start");

    DisplayF.initialize();

    sNeedRefreshData = true;
    Header();
    Menu();
}
//--------------------------------------------------------------------------------
void loop()
{
    char c;

    if(Serial.available())
    {
        c = Serial.read();
        Menu();
        switch(c)
        {
            case '+':
                if(suDisplayValue < 999)
                {
                    suDisplayValue++;
                    DisplayF.setDisplayValue(suDisplayValue);
                }
                sNeedRefreshData = true;

                break;
            case '-':
                if(suDisplayValue > 0)
                {
                    suDisplayValue--;
                    DisplayF.setDisplayValue(suDisplayValue);
                }
                sNeedRefreshData = true;
                break;

            case '*':
                suRefreshFreq = suRefreshFreq + 1;
                DisplayF.setRefreshFreq(suRefreshFreq);
                sNeedRefreshData = true;
                break;
            case '/':
                suRefreshFreq = suRefreshFreq - 1;
                DisplayF.setRefreshFreq(suRefreshFreq);
                sNeedRefreshData = true;
                break;

            case 'g':
                sbGreenLed = false;
                DisplayF.setGreenLed(sbGreenLed);
                sNeedRefreshData = true;
                break;
            case 'G':
                sbGreenLed = true;
                DisplayF.setGreenLed(sbGreenLed);
                sNeedRefreshData = true;
                break;


            case 'r':
                sbRedLed = false;
                DisplayF.setRedLed(sbRedLed);
                sNeedRefreshData = true;
                break;
            case 'R':
                sbRedLed = true;
                DisplayF.setRedLed(sbRedLed);
                sNeedRefreshData = true;
                break;

        }
    } // if serial available
    else
        IdleProc();
}
//-----------------------------------------------------------
void IdleProc()
{

    DisplayF.refresh(); // loop function for refresh display

    if(sNeedRefreshData)
    {
        sNeedRefreshData = false;
        GotoXY(1,3);
        ClearLine(); Serial.print(" Display Refresh freq      = ");
        SetColor(Yellow,true,true);  Serial.print(suRefreshFreq); SetColor(Reset); Serial.println("");
        ClearLine(); Serial.print(" Display Value      = ");
        SetColor(Yellow,true,true);  Serial.print(suDisplayValue); SetColor(Reset); Serial.println("");
        GotoXY(40,3); Serial.print(" Led Green                 = ");
        SetColor(Yellow,true,true);  Serial.print((sbGreenLed) ? "ON" : "Off"); SetColor(Reset); Serial.println("");
        GotoXY(40,4); Serial.print(" Led Red                   = ");
        SetColor(Yellow,true,true);  Serial.print((sbRedLed) ? "ON" : "Off"); SetColor(Reset); Serial.println("");
    }
}

//-----------------------------------------------------------
void Header()
{
    ClearScreen();
    Serial.println("Display Fagor control test ");
    Serial.println("===========================");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");

}
//-----------------------------------------------------------
void  Menu()
{
    GotoXY(0,8);
    Serial.println("----------------------------------------------------------------------------");
    Serial.println(" +   Increase value Display               -   Decrease value Display");
    Serial.println(" G/g Led Green (On/off)                   R/r Led Red   (On/off)");
    Serial.println(" *   Increase refresh freq                /   Decrease refresh freq");
    Serial.println("----------------------------------------------------------------------------");
}
