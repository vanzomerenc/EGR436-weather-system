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

int draw_weather_station_ui(struct weather_station_status status)
{
    static struct weather_station_status prev_status = {0};

    // TODO:
    //   only update parts of status which have changed,
    //   to avoid display flickering

    ST7735_FillScreen(0);

    gui_draw(LIGHTING_ICON_POS, images[status.lighting]);
    GUI_PRINT_FORMATTED(TIME_TEXT_POS, "%02d:%02d", status.time.hour, status.time.min);
    GUI_PRINT_FORMATTED(DATE_TEXT_POS, "%02d/%02d/%04d", status.time.month, status.time.date, status.time.year);
    gui_print(OUTSIDE_HEADER_TEXT_POS, "OUTSIDE");
    gui_print(INSIDE_HEADER_TEXT_POS, "INSIDE");
    GUI_PRINT_FORMATTED(OUTSIDE_TEMPERATURE_TEXT_POS, "%2.f\xa7", status.outdoor_temperature);
    GUI_PRINT_FORMATTED(INSIDE_TEMPERATURE_TEXT_POS, "%2.f\xa7", status.indoor_temperature);
    GUI_PRINT_FORMATTED(OUTSIDE_HUMIDITY_TEXT_POS, "%2.f%%", status.outdoor_humidity);
    GUI_PRINT_FORMATTED(INSIDE_HUMIDITY_TEXT_POS, "%2.f%%", status.indoor_humidity);
    GUI_PRINT_FORMATTED(BAROMETER_TEXT_POS, "%2.1f \"Hg", status.pressure);

    prev_status = status;

    return 0;
}
