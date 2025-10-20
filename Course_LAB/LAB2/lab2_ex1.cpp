#include "tpl_os.h"
#include "Arduino.h"
#include <stdlib.h>
#define PIN_INPUT  A0
#define PIN_OUTPUT 12

unsigned short int activationTimes = 0;
unsigned char *value;
void setup()
{
    digitalWrite(13, LOW);
    pinMode(PIN_OUTPUT, OUTPUT);
}

TASK(TaskC)
{
    static uint8_t press_count = 0;
    uint16_t a0_val = analogRead(A0);
    uint8_t s = digitalRead(12);
    uint8_t pressed_long;
    if (s == HIGH)
    {
        if (press_count < 10)
        {
            press_count++;
        }
    }
    else
    {
        press_count = 0;
    }
    pressed_long = (press_count >= 10) ? 1 : 0;

  uint16_t msg = (a0_val & 0x3FF) | ((pressed_long ? 1 : 0) << 12);

  SendMessage(msg_sensor, &msg);

  TerminateTask();

}
TASK(TaskM)
{
    static uint8_t ref_set = 0;
    static uint16_t R = 0;

    uint16_t sensor_msg;
    uint16_t a0_val;
    uint8_t pressed_long;
    uint16_t X;
    uint8_t led_cmd;

    
    ReceiveMessage(msg_sensor_receive, &sensor_msg);

    a0_val = sensor_msg & 0x03FF; // bits 0..9
    pressed_long = (sensor_msg & (1 << 12)) ? 1 : 0;

    // 长按 -> 更新参考值
    if (pressed_long)
    {
        R = a0_val;
        ref_set = 1;
    }

    // 根据是否有参考值决定 LED 行为
    if (!ref_set)
    {
        led_cmd = 3; // LED 常亮
    }
    else
    {
        X = (a0_val > R) ? (a0_val - R) : (R - a0_val);

        if (X < 100)
            led_cmd = 0; // LED 关
        else if (X < 200)
            led_cmd = 1; // 慢闪
        else
            led_cmd = 2; // 快闪
    }

    // 发送命令给 TaskV
    SendMessage(msg_led, &led_cmd);
    TerminateTask();
}

TASK(TaskV)
{
    static uint8_t current_mode = 0; // 0..3
    static uint8_t led_state = LOW;  // 当前 LED 电平
    static uint8_t blink_count = 0;  // 计数器，用于慢闪

    uint8_t new_mode;

    // 尝试接收 TaskM 发来的最新命令
    StatusType ret = ReceiveMessage(msg_led_receive, &new_mode);
    if (ret == E_OK)
    {
        current_mode = new_mode;                      // 更新 LED 模式
        blink_count = 0;                              // 重置计数器
        led_state = (current_mode == 3) ? HIGH : LOW; // 常亮直接 HIGH
        digitalWrite(13, led_state);
    }

    // 根据当前模式驱动 LED
    switch (current_mode)
    {
    case 0: // LED 关
        digitalWrite(13, LOW);
        break;

    case 1: // 慢闪 1 Hz → 半周期 500 ms → 125 ms tick 下每 4 tick toggle
        blink_count++;
        if (blink_count >= 4)// 4 * 125ms = 500ms
        { 
            led_state = !led_state;
            digitalWrite(13, led_state);
            blink_count = 0;
        }
        break;

    case 2: // 快闪 4 Hz → 半周期 125 ms → 每个 tick toggle
        led_state = !led_state;
        digitalWrite(13, led_state);
        break;

    case 3: // 常亮
        digitalWrite(13, HIGH);
        break;

    default:
        digitalWrite(13, LOW); // 安全措施
        break;
    }

    TerminateTask();
}