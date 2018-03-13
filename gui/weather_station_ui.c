#include "weather_station_ui.h"
#include "gui_layout.h"
#include "embedded_gui.h"

//
// IMAGES
//
// TODO: put these somewhere else

#include "../resources/image_dark.txt"
#include "../resources/image_twilight.txt"
#include "../resources/image_overcast.txt"
#include "../resources/image_partly_sunny.txt"
#include "../resources/image_sunny.txt"

static uint16_t const *const images[5] =
{
 image_dark,
 image_twilight,
 image_overcast,
 image_partly_sunny,
 image_sunny
};


static struct weather_station_status trend_old_values = {0};
static int trend_countdowns[5] = {0};
void draw_trend(struct text_placement location, float *old, float new, float eps, int index)
{
    if(new == 0) {return;}
    if(*old == 0) {*old = new; return;}
    if(new - *old > eps)
    {
        gui_print(location, "\x1E");
        *old = new;
        trend_countdowns[index] = 10;
    }
    else if(new - *old < -eps)
    {
        gui_print(location, "\x1F");
        *old = new;
        trend_countdowns[index] = 10;
    }
    else if(trend_countdowns[index] > 0)
    {
        trend_countdowns[index]--;
    }
    else
    {
        gui_print(location, " ");
    }
}

int draw_weather_station_ui(struct weather_station_status status)
{
    gui_draw(LIGHTING_ICON_POS, images[status.lighting]);
    GUI_PRINT_FORMATTED(TIME_TEXT_POS, "%02d:%02d", status.time.hour, status.time.min);
    GUI_PRINT_FORMATTED(DATE_TEXT_POS, "%02d/%02d", status.time.month, status.time.date);
    gui_print(OUTSIDE_HEADER_TEXT_POS, "OUTSIDE");
    gui_print(INSIDE_HEADER_TEXT_POS, "INSIDE");

    GUI_PRINT_FORMATTED(OUTSIDE_TEMPERATURE_VALUE_POS, "%2.f", status.outdoor_temperature);
    gui_print(OUTSIDE_TEMPERATURE_LABEL_POS, "\xF7""F");
    draw_trend(OUTSIDE_TEMPERATURE_TREND_POS, &trend_old_values.outdoor_temperature, status.outdoor_temperature, 1.0f, 0);

    GUI_PRINT_FORMATTED(INSIDE_TEMPERATURE_VALUE_POS, "%2.f", status.indoor_temperature);
    gui_print(INSIDE_TEMPERATURE_LABEL_POS, "\xF7""F");
    draw_trend(INSIDE_TEMPERATURE_TREND_POS, &trend_old_values.indoor_temperature, status.indoor_temperature, 1.0f, 1);

    GUI_PRINT_FORMATTED(OUTSIDE_HUMIDITY_VALUE_POS, "%2.f", status.outdoor_humidity);
    gui_print(OUTSIDE_HUMIDITY_LABEL_POS, "%");
    draw_trend(OUTSIDE_HUMIDITY_TREND_POS, &trend_old_values.outdoor_humidity, status.outdoor_humidity, 1.0f, 2);

    GUI_PRINT_FORMATTED(INSIDE_HUMIDITY_VALUE_POS, "%2.f", status.indoor_humidity);
    gui_print(INSIDE_HUMIDITY_LABEL_POS, "%");
    draw_trend(INSIDE_HUMIDITY_TREND_POS, &trend_old_values.indoor_humidity, status.indoor_humidity, 1.0f, 3);

    GUI_PRINT_FORMATTED(BAROMETER_VALUE_POS, "%2.2f", status.pressure);
    gui_print(BAROMETER_LABEL_POS, "inHg");
    draw_trend(BAROMETER_TREND_POS, &trend_old_values.pressure, status.pressure, 0.01f, 4);

    return 0;
}
