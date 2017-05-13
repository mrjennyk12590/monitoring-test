#include "Monitor.h"
#include "ev3api.h"

Monitor::Monitor()
{
    reset();
}

Monitor::~Monitor()
{
}

void Monitor::reset()
{
    x_ = 0;
    y_ = 0;
    ev3_lcd_fill_rect(x_, y_, EV3_LCD_WIDTH, EV3_LCD_HEIGHT, EV3_LCD_WHITE);
    ev3_lcd_set_font(EV3_FONT_SMALL);
}

void Monitor::display(const char* str)
{
    next();
    ev3_lcd_draw_string(str, x_, y_);
}

void Monitor::next()
{
    x_ = 0;
    y_ += 8;
}
