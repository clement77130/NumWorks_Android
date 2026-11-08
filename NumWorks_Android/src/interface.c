#include "interface.h"

static int isq(int v) { int r = 0; while ((r + 1) * (r + 1) <= v) r++; return r; }

void ui_fill(int x, int y, int w, int h, uint16_t c) {
  if (x < 0) { w += x; x = 0; }
  if (y < 0) { h += y; y = 0; }
  if (x + w > SCREEN_W) w = SCREEN_W - x;
  if (y + h > SCREEN_H) h = SCREEN_H - y;
  if (w <= 0 || h <= 0) return;
  eadk_rect_t r = {(uint16_t)x, (uint16_t)y, (uint16_t)w, (uint16_t)h};
  eadk_display_push_rect_uniform(r, c);
}

static int row_inset(int r, int jj) {
  int yy = r - 1 - jj;
  int s = isq(4 * r * r - (2 * yy + 1) * (2 * yy + 1));
  return r - (s + 1) / 2;
}

void ui_round(int x, int y, int w, int h, int r, uint16_t c) {
  if (r * 2 > h) r = h / 2;
  if (r * 2 > w) r = w / 2;
  if (r <= 0) { ui_fill(x, y, w, h, c); return; }
  for (int j = 0; j < r; j++) {
    int in = row_inset(r, j);
    ui_fill(x + in, y + j, w - 2 * in, 1, c);
    ui_fill(x + in, y + h - 1 - j, w - 2 * in, 1, c);
  }
  ui_fill(x, y + r, w, h - 2 * r, c);
}

void ui_line(int x0, int y0, int x1, int y1, int th, uint16_t c) {
  int dx = x1 > x0 ? x1 - x0 : x0 - x1, sx = x0 < x1 ? 1 : -1;
  int dy = -(y1 > y0 ? y1 - y0 : y0 - y1), sy = y0 < y1 ? 1 : -1;
  int err = dx + dy;
  for (;;) {
    ui_fill(x0, y0, th, th, c);
    if (x0 == x1 && y0 == y1) break;
    int e2 = 2 * err;
    if (e2 >= dy) { err += dy; x0 += sx; }
    if (e2 <= dx) { err += dx; y0 += sy; }
  }
}

void ui_text(const char *s, int x, int y, int large, uint16_t fg, uint16_t bg) {
  eadk_point_t p = {(uint16_t)x, (uint16_t)y};
  eadk_display_draw_string(s, p, large != 0, fg, bg);
}

void ui_background(int x, int y, int w, int h) {
  if (x < 0) { w += x; x = 0; }
  if (y < 0) { h += y; y = 0; }
  if (x + w > SCREEN_W) w = SCREEN_W - x;
  if (y + h > SCREEN_H) h = SCREEN_H - y;
  if (w <= 0 || h <= 0) return;
  for (int yy = y; yy < y + h; yy++) {
    if (wallpaper_present) {
      eadk_rect_t r = {(uint16_t)x, (uint16_t)yy, (uint16_t)w, 1};
      eadk_display_push_rect(r, &wallpaper_pixels[yy * SCREEN_W + x]);
    } else if (g_settings.theme == 2) {
      ui_fill(x, yy, w, 1, res_theme()->bg);
    } else {
      int R = 30 + (8 - 30) * yy / SCREEN_H, G = 45 + (8 - 45) * yy / SCREEN_H, B = 90 + (16 - 90) * yy / SCREEN_H;
      ui_fill(x, yy, w, 1, RGB565(R, G, B));
    }
  }
}

void ui_titlebar(const char *title) {
  const theme_t *t = res_theme();
  ui_fill(0, 0, SCREEN_W, 24, t->bar);
  ui_text("<", 8, 3, 1, res_accent(), t->bar);
  ui_text(title, 26, 3, 1, t->text, t->bar);
  nwclock_t c; char b[8];
  clock_get(&c); ru_time_str(&c, b);
  ui_text(b, SCREEN_W - 5 * 7 - 6, 5, 0, t->sub, t->bar);
}

void ui_navbar(void) {
  const theme_t *t = res_theme();
  ui_fill(0, 218, SCREEN_W, 22, t->bar);
  ui_round(75, 224, 11, 11, 3, t->sub);            /* recents */
  ui_round(77, 226, 7, 7, 1, t->bar);
  ui_round(154, 223, 13, 13, 6, t->sub);           /* accueil */
  for (int j = -5; j <= 5; j++) {                  /* retour */
    int a = j < 0 ? -j : j;
    ui_fill(235 + a, 229 + j, 12 - a, 1, t->sub);
  }
}

static const uint8_t segs[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

static void big_digit(int d, int x, int y, uint16_t c) {
  const int w = 16, h = 28, t = 3, hh = (h - 3 * t) / 2;
  uint8_t s = segs[d];
  if (s & 1)  ui_fill(x + t, y, w - 2 * t, t, c);
  if (s & 2)  ui_fill(x + w - t, y + t, t, hh, c);
  if (s & 4)  ui_fill(x + w - t, y + 2 * t + hh, t, hh, c);
  if (s & 8)  ui_fill(x + t, y + h - t, w - 2 * t, t, c);
  if (s & 16) ui_fill(x, y + 2 * t + hh, t, hh, c);
  if (s & 32) ui_fill(x, y + t, t, hh, c);
  if (s & 64) ui_fill(x + t, y + t + hh, w - 2 * t, t, c);
}

void ui_big_time(int h, int m, int x, int y, uint16_t c) {
  big_digit(h / 10, x, y, c);
  big_digit(h % 10, x + 20, y, c);
  ui_fill(x + 41, y + 8, 4, 4, c);
  ui_fill(x + 41, y + 18, 4, 4, c);
  big_digit(m / 10, x + 50, y, c);
  big_digit(m % 10, x + 70, y, c);
}

void ui_icon(int id, int x, int y, uint16_t tile) {
  const uint16_t W = WHITE;
  switch (id) {
    case 0: /* calculatrice */
      ui_round(x + 6, y + 6, 11, 11, 3, W);  ui_round(x + 23, y + 6, 11, 11, 3, W);
      ui_round(x + 6, y + 23, 11, 11, 3, W); ui_round(x + 23, y + 23, 11, 11, 3, W);
      break;
    case 1: /* graphe */
      ui_fill(x + 8, y + 6, 2, 28, W); ui_fill(x + 8, y + 32, 26, 2, W);
      ui_line(x + 11, y + 28, x + 18, y + 20, 2, W);
      ui_line(x + 18, y + 20, x + 24, y + 24, 2, W);
      ui_line(x + 24, y + 24, x + 33, y + 10, 2, W);
      break;
    case 2: ui_text("f(x)", x + 6, y + 13, 0, W, tile); break;
    case 3: /* tableur */
      ui_round(x + 6, y + 6, 28, 28, 3, W);
      for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) ui_fill(x + 8 + 9 * i, y + 8 + 9 * j, 7, 7, tile);
      break;
    case 4: /* stats */
      ui_fill(x + 8, y + 22, 7, 12, W); ui_fill(x + 18, y + 12, 7, 22, W); ui_fill(x + 28, y + 17, 7, 17, W);
      break;
    case 5: /* regressions */
      ui_line(x + 7, y + 30, x + 33, y + 10, 2, W);
      ui_fill(x + 9, y + 25, 4, 4, W); ui_fill(x + 16, y + 21, 4, 4, W);
      ui_fill(x + 22, y + 22, 4, 4, W); ui_fill(x + 29, y + 11, 4, 4, W);
      break;
    case 6: /* suites */
      ui_fill(x + 6, y + 28, 8, 6, W); ui_fill(x + 14, y + 22, 8, 12, W);
      ui_fill(x + 22, y + 16, 8, 18, W); ui_fill(x + 30, y + 10, 8, 24, W);
      break;
    case 7: /* probabilites : de */
      ui_round(x + 6, y + 6, 28, 28, 6, W);
      ui_fill(x + 11, y + 11, 4, 4, tile); ui_fill(x + 25, y + 11, 4, 4, tile);
      ui_fill(x + 18, y + 18, 4, 4, tile);
      ui_fill(x + 11, y + 25, 4, 4, tile); ui_fill(x + 25, y + 25, 4, 4, tile);
      break;
    case 8: ui_text("Py", x + 10, y + 11, 1, W, tile); break;
    case 9: /* parametres : curseurs */
      ui_fill(x + 6, y + 11, 28, 3, W); ui_fill(x + 6, y + 20, 28, 3, W); ui_fill(x + 6, y + 29, 28, 3, W);
      ui_round(x + 22, y + 8, 8, 9, 3, W); ui_round(x + 10, y + 17, 8, 9, 3, W); ui_round(x + 24, y + 26, 8, 9, 3, W);
      break;
  }
}
