#define Q_SIZE 5
#define PIN_A0 (14)

#include "tpl_os.h"
#include "Arduino.h"
#include <stdlib.h>
#include "Queue_Process.h"

enum LedState_T
{
    LED_OFF = 0,
    LED_SLOW_BLINK,
    LED_FAST_BLINK,
    LED_ON
};
void LED_mode_select(LedState_T mode);


static int error = 0;   /* Written by S */
static int alarm = 0;   /* Written by B */

/* 全局队列（受资源 QueueRes 保护） */
SampleQueue_t ADCval_Queue = 
{   
    .buffer = {0},
    .head = 0,
    .tail = 0,
    .count = 0
};

void setup()
{
    digitalWrite(13, LOW);
    Serial.begin(115200);
}

TASK(TaskW)   // 100ms periodic (merged S + B)
{
    static int activation_count = 0;   // To track every 500ms
    int overflow_flag = 0;

    GetResource(RES_SHARED);
    
    activation_count++;

    /* --------------------  S part  (every 100ms) -------------------- */

    uint16_t a0_val = analogRead(PIN_A0);
    overflow_flag = enqueue_sample(&ADCval_Queue, a0_val);
    error = (a0_val < 10 || a0_val > 1013) ? 1 : 0; // Update error flag

    ReleaseResource(RES_SHARED);

    if (overflow_flag == 1)
    {
        Serial.println("QUEUE OVERFLOW");
    }

    /* --------------------  B part  (every 500ms) -------------------- */
    if (activation_count % 5 == 0)   // 5 * 100ms = 500ms
    {
        int samples[Q_SIZE] = {0};
        int local_min, local_max;
        int pick_val = 0;

        GetResource(RES_SHARED);

        int n = dequeue_all(&ADCval_Queue, samples);

        if (n != 0)
        {
            local_min = samples[0];
            local_max = samples[0];

            for (int i = 1; i < n; i++)
            {
                if (samples[i] < local_min) local_min = samples[i];
                if (samples[i] > local_max) local_max = samples[i];
            }

            pick_val = local_max - local_min;
            alarm = (pick_val > 500) ? 1 : 0;
        }
        else
        {
            alarm = 0;
        }
        ReleaseResource(RES_SHARED);

        Serial.print("pick_val:");
        Serial.println(pick_val);

        
    }

    TerminateTask();
}

TASK(TaskV) //125ms
{
    static LedState_T current_mode = LED_OFF;
    int local_error, local_alarm;

    GetResource(RES_SHARED);
    local_error = error;
    local_alarm = alarm;
    ReleaseResource(RES_SHARED);

    current_mode = (local_error == 1)                     ? LED_FAST_BLINK :
                   (local_error == 0 && local_alarm == 1) ? LED_SLOW_BLINK :
                   (local_error == 0 && local_alarm == 0) ? LED_OFF :
                                                            LED_OFF;  
    LED_mode_select(current_mode);

    TerminateTask();
}

void LED_mode_select(LedState_T mode)
{
    static uint8_t led_state = LOW;  // 当前 LED 电平
    static uint8_t blink_count = 0;  // 计数器，用于慢闪

    switch (mode)// 根据当前模式驱动 LED
    {
    case LED_OFF: // LED 关
        digitalWrite(13, LOW);
        break;

    case LED_SLOW_BLINK: // 慢闪 1 Hz → 半周期 500 ms → 125 ms tick 下每 4 tick toggle
        blink_count++;
        if (blink_count >= 4)// 4 * 125ms = 500ms
        { 
            led_state = !led_state;
            digitalWrite(13, led_state);
            blink_count = 0;
        }
        break;

    case LED_FAST_BLINK: // 快闪 4 Hz → 半周期 125 ms → 每个 tick toggle
        led_state = !led_state;
        digitalWrite(13, led_state);
        break;

    case LED_ON: // 常亮
        digitalWrite(13, HIGH);
        break;

    default:
        digitalWrite(13, LOW);
        break;
    }
}