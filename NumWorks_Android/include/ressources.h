#ifndef RESSOURCES_H
#define RESSOURCES_H
#include <stdint.h>
#include <stddef.h>
#include "eadk.h"

#define SCREEN_W 320
#define SCREEN_H 240
#define NWOS_VERSION "1.0.0"
#define RGB565(r,g,b) ((uint16_t)((((r)>>3)<<11)|(((g)>>2)<<5)|((b)>>3)))
#define WHITE RGB565(255,255,255)

typedef struct { const char *name; uint16_t bg, card, bar, text, sub; } theme_t;
typedef struct { int theme, accent, navbar, key_repeat; } settings_t;
typedef struct { int h, m, d, mo, y; } nwclock_t;

extern settings_t g_settings;
extern const uint16_t wallpaper_pixels[];   /* genere par tools/prepare_assets.py */
extern const int wallpaper_present;

const theme_t *res_theme(void);
const theme_t *res_theme_at(int i);
int res_theme_count(void);
uint16_t res_accent(void);
const char *res_accent_name(void);
int res_accent_count(void);

void clock_init(void);
void clock_set(int h, int m, int d, int mo, int y);
void clock_get(nwclock_t *c);
int clock_weekday(const nwclock_t *c);
int clock_days_in_month(int mo, int y);
const char *res_month_name(int mo);
const char *res_day_name(int d);

int ru_strlen(const char *s);
void ru_append(char *dst, const char *s);
void ru_append_int(char *dst, int v);
void ru_time_str(const nwclock_t *c, char *out);
void ru_fmt_double(double v, char *out);   /* out >= 32 octets */
#endif
