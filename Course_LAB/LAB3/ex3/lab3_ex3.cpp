#include "tpl_os.h"
#include "Arduino.h"
#include <stdlib.h>
typedef unsigned long u64 ;

void MY_serial_print(u64 start, u64 end, u64* max_resp_C);

uint8_t resourceUsed = 0;

void do_things(int ms)
{
    unsigned long mul = ms * 504UL;
    for (unsigned long i = 0; i < mul; i++)
    {
        millis();
    }
}

void unlockResource(uint8* send_msg)
{
    *send_msg = 0;
    SendMessage(msgDataSend, send_msg);
}
void blockResource(uint8* rece_msg)
{
    while(*rece_msg)
    {
        ReceiveMessage(msgDataReceiveUnqueued, rece_msg);
    }
    *rece_msg = 1;
}

void setup()
{   
    Serial.begin(115200);
    uint8_t resource_init = 1;
    SendMessage(msgDataSend, &resource_init);
}

TASK(TaskA)
{
    u64 start = 0, end = 0;

    blockResource(&resourceUsed);

    start = millis();
    Serial.print("[A]start = ");
    Serial.println(start);

    do_things(200);
    unlockResource(&resourceUsed);

    end = millis();
    Serial.print("[A]end   = ");
    Serial.println(end);

    TerminateTask();
}

TASK(TaskB) 
{
    u64 start = 0, end = 0;

    start = millis();
    Serial.print("[B]start = ");
    Serial.println(start);

    do_things(700);

    end = millis();
    Serial.print("[B]end   = ");
    Serial.println(end);

    TerminateTask();
}


TASK(TaskC)
{
    static u64 max_resp_C = 0;
    u64 start = 0, end = 0;

    start = millis();
    Serial.print("[C]start = ");
    Serial.println(start);

    do_things(97);

    blockResource(&resourceUsed);
    
    Serial.print("[C]block = ");
    start = millis();
    Serial.println(start);

    do_things(205);

    unlockResource(&resourceUsed);

    end = millis();
    Serial.print("[C]end   = ");
    Serial.println(end);

    MY_serial_print(start, end, &max_resp_C);

    TerminateTask();
}
    
void MY_serial_print(u64 start, u64 end, u64* max_resp_C)
{
    u64 resp = 0;
    static u64 activation_count = 0;

    activation_count++;
    resp = end - start;

    if (resp > *max_resp_C + 10)
    {
        *max_resp_C = resp;
        Serial.print("[C] New max response: ");
        Serial.println(*max_resp_C);
    }
}