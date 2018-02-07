/*
 * gui_layout.h
 *
 *  Created on: Jan 20, 2018
 *      Author: chris
 */

#ifndef GUI_LAYOUT_H_
#define GUI_LAYOUT_H_

//
// STATUS DISPLAY
//

#define LIGHTING_ICON_POS            (struct bitmap_placement) {100, 24, 24, 24}

#define OUTSIDE_COL 2
#define INSIDE_COL 12

#define TIME_TEXT_POS                (struct text_placement) {0, 0, 3}
#define DATE_TEXT_POS                (struct text_placement) {0, 3, 2}
#define OUTSIDE_HEADER_TEXT_POS      (struct text_placement) {OUTSIDE_COL, 6, 1}
#define INSIDE_HEADER_TEXT_POS       (struct text_placement) {INSIDE_COL, 6, 1}
#define OUTSIDE_TEMPERATURE_TEXT_POS (struct text_placement) {OUTSIDE_COL, 7, 2}
#define INSIDE_TEMPERATURE_TEXT_POS  (struct text_placement) {INSIDE_COL, 7, 2}
#define OUTSIDE_HUMIDITY_TEXT_POS    (struct text_placement) {OUTSIDE_COL, 9, 2}
#define INSIDE_HUMIDITY_TEXT_POS     (struct text_placement) {INSIDE_COL, 9, 2}
#define BAROMETER_TEXT_POS           (struct text_placement) {2, 12, 2}

//
// MENU DISPLAY
//
#define MENU_TITLE_TEXT_POS (struct text_placement) {0, 0, 3}
#define MENU_ITEM_POS(ITEM_NO) (struct text_placement) {2, 4 + 2*(ITEM_NO), 2}


#endif /* GUI_LAYOUT_H_ */
