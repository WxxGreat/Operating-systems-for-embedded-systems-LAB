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

static shared_data_t shared = { {0}, 0, 0 };

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

uint8_t is_high(void)
{
    return digitalRead(13) == HIGH ? 1 : 0;
}

void setup()
{
    pinMode(13, INPUT);
    Serial.begin(115200);
    timerInit();
}

TASK(TaskS) //1ms
{
    // State machine variables (kept across invocations)
    static bool prev_high = false;       // previous sampled pin state
    static uint16_t high_ms = 0;         // current is_high pulse length in ms
    static uint16_t low_ms = 0;          // current low length in ms
    static uint8_t pulse_count = 0;      // pulses counted for current digit
    static bool digit_error = false;     // whether any pulse in digit had width error
    static bool digit_finalized = false; // whether current digit was already printed
    static bool printed_slash = false;   // whether '/' was printed for current number
    bool high = is_high();

    if (high) // in is_high pulse
    {
        if (!prev_high) // rising edge
        {
            // starting a new pulse after a long low means new number/digit started
            if (low_ms >= 200) printed_slash = false;

            if (low_ms >= 100 && low_ms < 200) // digit separator: low gap between 100 and 200 ms
            {
                if (!digit_finalized && pulse_count > 0)
                {
                    // finalize digit and append directly to shared buffer
                    uint8_t digit = (pulse_count == 10) ? 0 : pulse_count;
                    uint16_t pkt = (uint16_t)digit;
                    if (digit_error) pkt |= PKT_ERR_FLAG;
                    GetResource(RES_PBUF);
                    if (shared.shared_count < (sizeof(shared.shared_buf)/sizeof(shared.shared_buf[0])))
                    {
                        shared.shared_buf[shared.shared_count++] = pkt;
                    }
                    else
                    {
                        shared.overflow = 1;
                    }
                    ReleaseResource(RES_PBUF);
                    // reset for next digit
                    pulse_count = 0;
                    digit_error = false;
                    digit_finalized = true;
                }
            }
        }

        high_ms++;
        // detect too long pulses early while still high
        if (high_ms > 60)
        {
            digit_error = true;
        }
    }
    else // in low pulse
    {
        if (prev_high) // falling edge -> end of a is_high pulse
        {
            low_ms = 0;
            pulse_count++;
            // short pulses (<40ms) are errors; long pulses (>60ms) are
            // already detected while still high (high_ms > 60), so
            // only check the short condition here to avoid duplicate checks.
            if (high_ms < 40)
            {
                digit_error = true;
            }
            high_ms = 0;// reset is_high counter
        }
        low_ms++;

        if (low_ms >= 200) // end of number: low gap >200ms
        {
            if (pulse_count > 0 && !digit_finalized)
            {
                // finalize last digit and append directly to shared buffer
                uint8_t digit = (pulse_count == 10) ? 0 : pulse_count;
                uint16_t pkt = (uint16_t)digit;
                if (digit_error) pkt |= PKT_ERR_FLAG;
                GetResource(RES_PBUF);
                if (shared.shared_count < (sizeof(shared.shared_buf)/sizeof(shared.shared_buf[0])))
                {
                    shared.shared_buf[shared.shared_count++] = pkt;
                }
                else
                {
                    shared.overflow = 1;
                }
                ReleaseResource(RES_PBUF);
                pulse_count = 0;
                digit_error = false;
            }
            if (!printed_slash)
            {
                // notify printer task to flush the shared buffer
                uint16_t pkt = PKT_SLASH;
                SendMessage(msg_print, &pkt);
                printed_slash = true;
            }
            digit_finalized = true;
        }
        // allow a new digit to be started when a rising edge happens
        if (!high && pulse_count == 0 && low_ms == 0)
        {
            digit_finalized = false;
        }
    }

    // on any rising edge we should allow building next digit
    if (high && !prev_high)
    {
        // reset digit_finalized so next low can trigger separators again
        digit_finalized = false;
        // new pulse started, new number if previously ended
        if (printed_slash) printed_slash = false;
    }

    prev_high = high;

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