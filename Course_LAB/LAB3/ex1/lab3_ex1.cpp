#include "tpl_os.h"
#include "Arduino.h"
#include <stdlib.h>


void do_things(int ms)
{
    unsigned long mul = ms * 504UL;
    unsigned long i;
    for(i=0; i<mul; i++) 
    {
        millis();
    }
}

void cleanOutput()
{
    digitalWrite(1, LOW);
    digitalWrite(2, LOW);
    digitalWrite(3, LOW);
}

void setup()
{
    Serial.begin(9600);
	pinMode(1, OUTPUT);
    pinMode(2, OUTPUT);
    pinMode(3, OUTPUT);
}

TASK(TaskA)
{
    digitalWrite(1, HIGH);
    do_things(200);
    digitalWrite(1, LOW);
    TerminateTask();
}

TASK(TaskB) 
{
    digitalWrite(2, HIGH);
    do_things(700);
    digitalWrite(2, LOW);
    TerminateTask();
}

TASK(TaskC) 
{
    static unsigned long max_response_C = 0;

    digitalWrite(3, HIGH);

    unsigned long start = millis();
    do_things(300);
    unsigned long end = millis();

    unsigned long response = end - start;
    if (response > max_response_C)
    {
        max_response_C = response;
    }

    Serial.print("Task C response: ");
    Serial.print(response);
    Serial.print(" ms | Max so far: ");
    Serial.print(max_response_C);
    Serial.println(" ms");

    digitalWrite(3, LOW);
    TerminateTask();
}