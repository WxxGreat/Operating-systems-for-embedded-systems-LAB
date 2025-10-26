#include "tpl_os.h"
#include "Arduino.h"
#include <stdlib.h>

#define PIN_A0 (14)

enum LedState_T
{
    LED_OFF = 0,
    LED_SLOW_BLINK,
    LED_FAST_BLINK,
    LED_ON
};

void setup()
{
    digitalWrite(13, LOW);
    pinMode(12, INPUT_PULLUP);
    Serial.begin(115200);
}

TASK(TaskC) //100ms
{
    static uint8_t press_count = 0;

    uint8_t pressed_long;
    uint16_t a0_val = analogRead(PIN_A0);

    static uint8_t last_pressed_long = 0;
    static uint16_t last_a0_val = 0;


    press_count = (digitalRead(12) == LOW) ? (press_count + 1) : 0;
    pressed_long = (press_count >= 10) ? 1 : 0; // 10 * 100ms = 1s

    if(pressed_long != last_pressed_long || a0_val != last_a0_val)
    {
        // bits 0..9: a0_val, bit 12: pressed_long
        uint16_t msg = (a0_val & 0x3FF) | ((pressed_long ? 1 : 0) << 12);
        SendMessage(msg_sensor, &msg);
    }

    last_pressed_long = pressed_long;
    last_a0_val = a0_val;

    TerminateTask();
}

TASK(TaskM) //Triggered by msg_sensor_receive
{
    static int16_t R = -1;

    uint16_t sensor_msg;
    uint16_t a0_val;
    uint8_t pressed_long;
    uint16_t X;
    LedState_T led_cmd;


    if (ReceiveMessage(msg_sensor_receive, &sensor_msg) == E_OK)
    {
        a0_val = sensor_msg & 0x03FF; // bits 0..9
        pressed_long = (sensor_msg & (1 << 12)) ? 1 : 0;

    
        if (pressed_long)// 长按 -> 更新参考值
        {
            R = a0_val;
        }

        if (R == -1) // R==-1 meaning no reference "R" set
        {
            led_cmd = LED_ON; // LED 常亮
        }
        else
        {
            X = abs((int16_t)a0_val - R);

            led_cmd = (X < 100) ? LED_OFF :
                    (X < 200) ? LED_SLOW_BLINK :
                                LED_FAST_BLINK;
        }

        Serial.print("X: ");
        Serial.println(X);
        Serial.print("led_cmd: ");
        Serial.println(led_cmd);

        SendMessage(msg_led, &led_cmd);// 发送命令给 TaskV
    }
    TerminateTask();
}

TASK(TaskV) //125ms
{
    static LedState_T current_mode = LED_OFF;
    static uint8_t led_state = LOW;  // 当前 LED 电平
    static uint8_t blink_count = 0;  // 计数器，用于慢闪

    uint8_t new_mode;
    StatusType ret = ReceiveMessage(msg_led_receive, &new_mode);
    if (ret == E_OK)
    {
        current_mode = new_mode == 0 ? LED_OFF :
                       new_mode == 1 ? LED_SLOW_BLINK :
                       new_mode == 2 ? LED_FAST_BLINK :
                       new_mode == 3 ? LED_ON : 
                                       LED_OFF;                     
    }

    switch (current_mode)// 根据当前模式驱动 LED
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

    TerminateTask();
}