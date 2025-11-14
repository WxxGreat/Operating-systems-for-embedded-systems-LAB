#include "tpl_os.h"
#include "Arduino.h"
#include <stdlib.h>

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

void cleanOutput()
{
    LED1(OFF);
    LED2(OFF);
    LED3(OFF);
}

void setup()
{
    Serial.begin(9600);
	pinMode(5, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(7, OUTPUT);
    cleanOutput();
}

TASK(TaskA)
{
    LED1(ON);
    do_things(200);
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
    unsigned long start = 0;
    unsigned long end = 0;
    unsigned long resp = 0;
    static unsigned long activation_count = 0;
    static unsigned long max_resp_C = 0;
    
    LED3(ON);

    activation_count++;

    start = millis();

    /* Simulate execution for 300 ms */
    do_things(300);

    end = millis();
    resp = end - start;

    if (resp > max_resp_C)
    {
        max_resp_C = resp;
        Serial.print("[TaskC] New max response: ");
        Serial.print(max_resp_C);
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
    LED3(OFF);
    /* Keep the task simple: terminate and wait for next alarm activation */
    TerminateTask();
}