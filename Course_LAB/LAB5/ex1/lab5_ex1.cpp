#include "tpl_os.h"
#include "Arduino.h"
#include <stdlib.h>
// Packet format macros
#define PKT_DIGIT_MASK 0x00FFu
#define PKT_ERR_FLAG   0x0100u
#define PKT_SLASH      0xFFFFu

// Messages declared in OIL
DeclareMessage(msg_print);
DeclareMessage(msg_print_receive);

// Shared buffer protected by RES_PBUF: written by TaskS at end-of-number,
// read by TaskV when printing
typedef struct
{
    uint16_t shared_buf[32];
    uint8_t shared_count;
    uint8_t overflow; /* set if buffer overflowed */
} shared_data_t;

typedef enum {
    ST_IDLE,          // 长时间低电平，等待第一个脉冲
    ST_HIGH,          // 正在高电平（计高脉冲宽度）
    ST_LOW_IN_DIGIT,  // 数字内部的低电平（~50ms）
    ST_LOW_GAP        // 数字间 or 号码结束的低电平
} fsm_state_t;

typedef struct
{
    fsm_state_t state;
    uint16_t high_ms;
    uint16_t low_ms;
    uint8_t  pulse_count;
    bool     digit_error;
} dial_fsm_t;
void dial_fsm_step(dial_fsm_t *fsm, bool high);

void timerInit() // interrupt each 1ms
{
    TCCR2A = 0;
    TCNT2 = 0;
    // clkio = 16MHz => clk = 250KHz
    TCCR2B = (1 << WGM22) | (1 << CS22); // CTC and clk = Clkio/64                      
    OCR2A = 250 - 1;
    TIMSK2 |= (1 << TOIE2);
    interrupts();
}

void setup()
{
    pinMode(13, INPUT);
    Serial.begin(115200);
    timerInit();
}

static shared_data_t shared = { {0}, 0, 0 };

uint8_t is_high(void)
{
    return digitalRead(13) == HIGH ? 1 : 0;
}

static void finalize_digit(uint8_t *pulse_count,bool *digit_error)
{
    if (*pulse_count == 0) return;

    uint8_t digit = (*pulse_count == 10) ? 0 : *pulse_count;
    uint16_t pkt  = digit;

    if (*digit_error) pkt |= PKT_ERR_FLAG;

    GetResource(RES_PBUF);
    if (shared.shared_count < 32)
        shared.shared_buf[shared.shared_count++] = pkt;
    else
        shared.overflow = 1;
    ReleaseResource(RES_PBUF);

    *pulse_count = 0;
    *digit_error = false;
}

static void finalize_number(void)
{
    uint16_t pkt = PKT_SLASH;
    SendMessage(msg_print, &pkt);
}


TASK(TaskS) // 1ms
{
    static dial_fsm_t dial_fsm = { ST_IDLE, 0, 0, 0, false };
    bool high = is_high();

    dial_fsm_step(&dial_fsm, high);

    TerminateTask();
}


TASK(TaskV) //125ms
{
    uint16_t pkt = 0;
    StatusType ret;
    // At end-of-number, TaskS sends PKT_SLASH and copies digits into shared_buf.
    // Here we only handle PKT_SLASH: copy shared buffer under RES_PBUF and print it.
    do {
        ret = ReceiveMessage(msg_print_receive, &pkt);
        if (ret == E_OK)
        {
            if (pkt == PKT_SLASH)
            {
                uint16_t local_buf[32];
                uint8_t local_count = 0;
                uint8_t local_overflow = 0;
                // copy shared buffer and overflow flag under protection
                GetResource(RES_PBUF);
                for (uint8_t i = 0; i < shared.shared_count && i < (sizeof(local_buf)/sizeof(local_buf[0])); ++i)
                {
                    local_buf[i] = shared.shared_buf[i];
                }
                local_count = shared.shared_count;
                local_overflow = shared.overflow;
                shared.shared_count = 0;
                shared.overflow = 0;
                ReleaseResource(RES_PBUF);

                // now print without holding resource
                for (uint8_t i = 0; i < local_count; ++i)
                {
                    uint8_t digit = local_buf[i] & PKT_DIGIT_MASK;
                    bool err = (local_buf[i] & PKT_ERR_FLAG) != 0;
                    Serial.print(digit);
                    if (err) Serial.print('*');
                }
                if (local_overflow) Serial.print('!');
                Serial.println("/");
            }
            else
            {
                // ignore unexpected packets — we only expect PKT_SLASH now
            }
        }
    } while (ret == E_OK);

    TerminateTask();
}


void dial_fsm_step(dial_fsm_t *fsm, bool high)
{
    switch (fsm->state)
    {
    case ST_IDLE:
        if (high)
        {
            fsm->high_ms = 1;
            fsm->state = ST_HIGH;
        }
        break;

    case ST_HIGH:
        if (high)
        {
            fsm->high_ms++;
            if (fsm->high_ms > 60)
            {
                fsm->digit_error = true;
            }
        }
        else
        {
            if (fsm->high_ms < 40)
            {
                fsm->digit_error = true;
            }

            fsm->pulse_count++;
            fsm->high_ms = 0;
            fsm->low_ms = 1;
            fsm->state = ST_LOW_IN_DIGIT;
        }
        break;

    case ST_LOW_IN_DIGIT:
        if (!high)
        {
            fsm->low_ms++;
            if (fsm->low_ms >= 100)
            {
                finalize_digit(&fsm->pulse_count, &fsm->digit_error);
                fsm->state = ST_LOW_GAP;
            }
        }
        else
        {
            fsm->high_ms = 1;
            fsm->state = ST_HIGH;
        }
        break;

    case ST_LOW_GAP:
        if (!high)
        {
            fsm->low_ms++;
            if (fsm->low_ms > 200)
            {
                finalize_number();
                fsm->state = ST_IDLE;
            }
        }
        else
        {
            fsm->low_ms = 0;
            fsm->high_ms = 1;
            fsm->state = ST_HIGH;
        }
        break;
    }
}
