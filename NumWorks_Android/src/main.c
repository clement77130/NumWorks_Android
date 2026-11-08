#include "applications.h"
#include "interface.h"
#include "navigation.h"

/* Metadonnees lues par Epsilon pour une appli externe */
const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "Android OS";
const uint32_t eadk_api_level __attribute__((section(".rodata.eadk_api_level"))) = 0;

#define COLS 5
#define CELL_W 64
#define ROW0_Y 96
#define ROW_DY 62

static void tile_pos(int i, int *x, int *y) { *x = (i % COLS) * CELL_W; *y = ROW0_Y + (i / COLS) * ROW_DY; }

static void draw_tile(int i, int sel) {
  const theme_t *t = res_theme();
  int cx, cy; tile_pos(i, &cx, &cy);
  ui_background(cx, cy - 3, CELL_W, 61);
  int ix = cx + 12, iy = cy + 2;
  if (sel) ui_round(ix - 3, iy - 3, 46, 46, 13, res_accent());
  ui_round(ix, iy, 40, 40, 10, g_apps[i].color);
  ui_icon(i, ix, iy, g_apps[i].color);
  const char *l = g_apps[i].label;
  int w = ru_strlen(l) * 7 + 8, px = cx + (CELL_W - w) / 2;
  uint16_t pill = sel ? res_accent() : t->bar;
  ui_round(px, cy + 43, w, 15, 7, pill);
  ui_text(l, px + 4, cy + 44, 0, sel ? WHITE : t->text, pill);
}

static void draw_clock(void) {
  const theme_t *t = res_theme();
  nwclock_t c; char d[32];
  clock_get(&c);
  ui_background(8, 22, 128, 66);
  ui_round(10, 24, 124, 62, 12, t->card);
  ui_big_time(c.h, c.m, 18, 30, t->text);
  d[0] = 0;
  ru_append(d, res_day_name(clock_weekday(&c))); ru_append(d, " ");
  ru_append_int(d, c.d); ru_append(d, " ");
  ru_append(d, res_month_name(c.mo)); ru_append(d, " ");
  ru_append_int(d, c.y);
  ui_text(d, 18, 64, 0, t->sub, t->card);
}

static void draw_home(int sel) {
  const theme_t *t = res_theme();
  ui_background(0, 0, SCREEN_W, SCREEN_H);
  ui_fill(0, 0, SCREEN_W, 16, t->bar);
  ui_text("NumWorks OS", 6, 1, 0, t->sub, t->bar);
  ui_text("N0120", SCREEN_W - 5 * 7 - 6, 1, 0, t->sub, t->bar);
  draw_clock();
  for (int i = 0; i < APP_COUNT; i++) draw_tile(i, i == sel);
  if (g_settings.navbar) ui_navbar();
}

int main(int argc, char *argv[]) {
  (void)argc; (void)argv;
  clock_init();
  int sel = 0;
  draw_home(sel);
  int last_min; { nwclock_t c; clock_get(&c); last_min = c.m; }
  for (;;) {
    int k = nav_poll();
    nwclock_t c; clock_get(&c);
    if (c.m != last_min) { last_min = c.m; draw_clock(); }
    if (k == NAV_NONE) continue;
    int old = sel;
    if (k == eadk_key_right) sel = (sel + 1) % APP_COUNT;
    else if (k == eadk_key_left) sel = (sel + APP_COUNT - 1) % APP_COUNT;
    else if (k == eadk_key_down) sel = (sel + COLS) % APP_COUNT;
    else if (k == eadk_key_up) sel = (sel + APP_COUNT - COLS) % APP_COUNT;
    else if (k == eadk_key_ok || k == eadk_key_exe) {
      g_apps[sel].run();
      draw_home(sel);
      continue;
    }
    else if (k == eadk_key_home) return 0;   /* quitte vers Epsilon */
    if (sel != old) { draw_tile(old, 0); draw_tile(sel, 1); }
  }
}
