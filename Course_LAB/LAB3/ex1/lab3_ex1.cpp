#pragma optimize( "", off) // Disable optimizations for this file
#include "tpl_os.h"
#include "Arduino.h"
#include <stdlib.h>

void MY_serial_print(unsigned long start, unsigned long end, unsigned long* max_resp_C);

#define ON  1
#define OFF 0
#define LED1(state) digitalWrite(5, (state == ON) ? OFF : ON)
#define LED2(state) digitalWrite(6, (state == ON) ? OFF : ON)
#define LED3(state) digitalWrite(7, (state == ON) ? OFF : ON)

void do_things(int ms)
{
    unsigned long mul = ms * 504UL;
    for (unsigned long i = 0; i < mul; i++)
    {
        millis();
    }
}

void setup()
{
	pinMode(5, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(7, OUTPUT);
    LED1(OFF);
    LED2(OFF);
    LED3(OFF);
    Serial.begin(115200);
}

TASK(TaskA)
{
    unsigned long start = 0, end = 0;

    start = millis();
    Serial.print("[TaskA]:start=");
    Serial.println(start);

    do_things(200);

    
    Serial.print("[TaskA]:end=");
    end = millis();
    Serial.println(end);

    TerminateTask();
}

TASK(TaskB) 
{
    unsigned long start = 0, end = 0;

    start = millis();
    Serial.print("[TaskB]:start=");
    Serial.println(start);

    do_things(700);

    Serial.print("[TaskB]:end=");
    end = millis();
    Serial.println(end);

    TerminateTask();
}


TASK(TaskC)
{
    unsigned long start = 0, end = 0;
    static unsigned long max_resp_C = 0;

    start = millis();
    Serial.print("[TaskC]:start=");
    Serial.println(start);

    do_things(300);

    Serial.print("[TaskC]:end=");
    end = millis();
    Serial.println(end);

    MY_serial_print(start, end, &max_resp_C);

    LED3(OFF);
    TerminateTask();
}

void MY_serial_print(unsigned long start, unsigned long end, unsigned long* max)
{
    unsigned long resp = 0;
    resp = end - start;

    if (resp > *max)
    {
        *max = resp;
        Serial.print("[TaskC] New max response: ");
        Serial.println(*max);
    }
}