#include "servo.h"
void servo_set(uint16_t angle)
{
    if(angle >180)angle = 180;
    tim_set_Pulse(500+(uint32_t)((angle/180.0f)*2000.0f));

}
