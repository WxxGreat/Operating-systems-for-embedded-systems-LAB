#include <stdio.h>
#include <math.h>
#include "tpl_os.h"

int main(void)
{
    StartOS(OSDEFAULTAPPMODE);
    return 0;
}

DeclareAlarm(a500msec);
DeclareAlarm(a750msec);
TASK(TaskA)
{
    static int occurenceA = 0;
    int valueA = occurenceA * 500;
    fprintf(stdout, "Task A: \t%d\r\n", valueA);
    occurenceA++;
    TerminateTask();
}
TASK(TaskB)
{
    static int occurenceB = 1500;
    fprintf(stdout, "\t\t\t\tTask B: \t%d\r\n", occurenceB);
    occurenceB += 750;
    TerminateTask();
}
TASK(stop)
{
    CancelAlarm(a500msec);
    CancelAlarm(a750msec);
    printf("Shutdown\r\n");
    ShutdownOS(E_OK);
    TerminateTask();
}