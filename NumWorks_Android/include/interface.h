#ifndef INTERFACE_H
#define INTERFACE_H
#include "ressources.h"

void ui_fill(int x, int y, int w, int h, uint16_t c);
void ui_round(int x, int y, int w, int h, int r, uint16_t c);
void ui_line(int x0, int y0, int x1, int y1, int th, uint16_t c);
void ui_text(const char *s, int x, int y, int large, uint16_t fg, uint16_t bg);
void ui_background(int x, int y, int w, int h);       /* fond d'ecran ou degrade */
void ui_titlebar(const char *title);
void ui_navbar(void);
void ui_big_time(int h, int m, int x, int y, uint16_t c);
void ui_icon(int id, int x, int y, uint16_t tile);    /* glyphe 40x40 de l'appli id */
#endif
