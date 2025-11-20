#pragma optimize( "", off)
#include "tpl_os.h"
#include "Arduino.h"
#include <stdlib.h>

DeclareResource(TaskA_C_Resource);

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

void unlockResource(uint8* resourceUsed)
{
    *resourceUsed=0;
    SendMessage(msgDataSend, resourceUsed);
}
void blockResource(uint8* resourceUsed)
{
    while(*resourceUsed)
    {
        ReceiveMessage(msgDataReceiveUnqueued, resourceUsed);
    }
    *resourceUsed=1;
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
    uint8_t resourceUsed = 0;
    SendMessage(msgDataSend, &resourceUsed);
}

TASK(TaskA)
{
    LED1(ON);
    uint8 resourceUsed=1;
    blockResource(&resourceUsed);
    do_things(200);
    unlockResource(&resourceUsed);
    LED1(OFF);
    TerminateTask();
}

TASK(TaskB) 
{
    LED2(ON);
    do_things(700);
    LED2(OFF);
    TerminateTask();
}


TASK(TaskC)
{
    static unsigned long max_resp_C = 0;
    unsigned long start = 0, end = 0;
    uint8_t resourceUsed=1;
    LED3(ON);
    start = millis();

    do_things(100);
    blockResource(&resourceUsed);
    do_things(200);
    end = millis();
    unlockResource(&resourceUsed);
    MY_serial_print(start, end, &max_resp_C);

    LED3(OFF);
    TerminateTask();
}
    
void MY_serial_print(unsigned long start, unsigned long end, unsigned long* max_resp_C)
{
    unsigned long resp = 0;
    static unsigned long activation_count = 0;

    activation_count++;
    resp = end - start;

    if (resp > *max_resp_C)
    {
        *max_resp_C = resp;
        Serial.print("[TaskC] New max response: ");
        Serial.print(*max_resp_C);
        Serial.println(" ms");
    }

    /* Print the last activation times and response for tracing */
    Serial.print("[TaskC] #");
    Serial.print(activation_count);
    Serial.print(" start=");
    Serial.print(start);
    Serial.print(" ms, end=");
    Serial.print(end);
    Serial.print(" ms, resp=");
    Serial.print(resp);
    Serial.println(" ms");
}