#pragma optimize( "", off) // Disable optimizations for this file
#include "tpl_os.h"
#include "Arduino.h"
#include <stdlib.h>

typedef unsigned long u64 ;

DeclareResource(TaskA_C_Resource);

void MY_serial_print(u64 start, u64 end, u64* max_resp_C);

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
    u64 start = 0, end = 0;

    GetResource(TaskA_C_Resource);

    Serial.print("[A]start = ");
    start = millis();
    Serial.println(start);

    do_things(200);

    Serial.print("[A]end   = ");
    end = millis();
    Serial.println(end);

    ReleaseResource(TaskA_C_Resource);
    TerminateTask();
}

TASK(TaskB) 
{
    do_things(700);
    TerminateTask();
}


TASK(TaskC)
{
    static unsigned long max_resp_C = 0;
    unsigned long start = 0, end = 0;

    Serial.print("[C]start = ");
    start = millis();
    Serial.println(start);

    do_things(100);
    GetResource(TaskA_C_Resource);

    do_things(200);
 
    Serial.print("[C]end   = ");
    end = millis();
    Serial.println(end);

    ReleaseResource(TaskA_C_Resource);

    MY_serial_print(start, end, &max_resp_C);

    //LED3(OFF);
    TerminateTask();
}
    
void MY_serial_print(u64 start, u64 end, u64* max_resp_C)
{
    u64 resp = 0;
    resp = end - start;

    if (resp > *max_resp_C)
    {
        *max_resp_C = resp;
        Serial.print("[TaskC] New max response: ");
        Serial.println(*max_resp_C);
    }
}