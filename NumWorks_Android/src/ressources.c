#include "ressources.h"

/* memcpy/memset fournis ici : l'edition de liens -r n'embarque pas toujours la libc */
void *memcpy(void *d, const void *s, size_t n) {
  char *a = (char *)d; const char *b = (const char *)s;
  while (n--) *a++ = *b++;
  return d;
}
void *memset(void *d, int c, size_t n) {
  char *a = (char *)d;
  while (n--) *a++ = (char)c;
  return d;
}

settings_t g_settings = {0, 0, 1, 1};

static const theme_t themes[3] = {
  {"Sombre", RGB565(20,21,26), RGB565(40,42,50), RGB565(12,12,15), RGB565(240,240,245), RGB565(150,153,168)},
  {"AMOLED", RGB565(0,0,0), RGB565(26,26,29), RGB565(0,0,0), RGB565(245,245,250), RGB565(140,142,155)},
  {"Clair",  RGB565(238,240,246), RGB565(255,255,255), RGB565(222,226,236), RGB565(25,27,35), RGB565(105,110,128)},
};
static const uint16_t accents[5] = {
  RGB565(61,125,255), RGB565(40,175,100), RGB565(255,149,0), RGB565(240,70,125), RGB565(150,100,255)
};
static const char *accent_names[5] = {"Bleu", "Vert", "Orange", "Rose", "Violet"};

const theme_t *res_theme(void) { return &themes[g_settings.theme]; }
const theme_t *res_theme_at(int i) { return &themes[i]; }
int res_theme_count(void) { return 3; }
uint16_t res_accent(void) { return accents[g_settings.accent]; }
const char *res_accent_name(void) { return accent_names[g_settings.accent]; }
int res_accent_count(void) { return 5; }

/* ---- Horloge logicielle : l'EADK n'a pas de RTC, l'heure se regle a la main ---- */
static uint64_t ref_ms;
static nwclock_t base = {12, 0, 1, 1, 2026};

void clock_init(void) { ref_ms = eadk_timing_millis(); }
void clock_set(int h, int m, int d, int mo, int y) {
  base.h = h; base.m = m; base.d = d; base.mo = mo; base.y = y;
  ref_ms = eadk_timing_millis();
}
int clock_days_in_month(int mo, int y) {
  static const int dm[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
  if (mo == 2 && ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0)) return 29;
  return dm[mo - 1];
}
void clock_get(nwclock_t *c) {
  uint32_t total = (uint32_t)base.h * 60u + (uint32_t)base.m + (uint32_t)((eadk_timing_millis() - ref_ms) / 60000u);
  uint32_t days = total / 1440u;
  total %= 1440u;
  c->h = (int)(total / 60u); c->m = (int)(total % 60u);
  c->d = base.d; c->mo = base.mo; c->y = base.y;
  while (days--) {
    if (++c->d > clock_days_in_month(c->mo, c->y)) {
      c->d = 1;
      if (++c->mo > 12) { c->mo = 1; c->y++; }
    }
  }
}
int clock_weekday(const nwclock_t *c) {
  static const int t[12] = {0,3,2,5,0,3,5,1,4,6,2,4};
  int y = c->y - (c->mo < 3);
  return (y + y/4 - y/100 + y/400 + t[c->mo - 1] + c->d) % 7; /* 0 = dimanche */
}
const char *res_month_name(int mo) {
  static const char *n[12] = {"janv","fevr","mars","avr","mai","juin","juil","aout","sept","oct","nov","dec"};
  return n[mo - 1];
}
const char *res_day_name(int d) {
  static const char *n[7] = {"Dim","Lun","Mar","Mer","Jeu","Ven","Sam"};
  return n[d];
}

/* ---- Utilitaires texte (sans libc) ---- */
int ru_strlen(const char *s) { int n = 0; while (s[n]) n++; return n; }
void ru_append(char *d, const char *s) { d += ru_strlen(d); while ((*d++ = *s++)) {} }
void ru_append_int(char *dst, int v) {
  char t[12]; int n = 0; unsigned u;
  dst += ru_strlen(dst);
  if (v < 0) { *dst++ = '-'; u = (unsigned)(-v); } else u = (unsigned)v;
  if (u == 0) t[n++] = '0';
  while (u) { t[n++] = (char)('0' + u % 10); u /= 10; }
  while (n) *dst++ = t[--n];
  *dst = 0;
}
void ru_time_str(const nwclock_t *c, char *o) {
  o[0] = (char)('0' + c->h / 10); o[1] = (char)('0' + c->h % 10); o[2] = ':';
  o[3] = (char)('0' + c->m / 10); o[4] = (char)('0' + c->m % 10); o[5] = 0;
}
void ru_fmt_double(double v, char *o) {
  int i = 0, k;
  o[0] = 0;
  if (v != v || v > 1e300 || v < -1e300) { ru_append(o, "Erreur"); return; }
  if (v < 0) { o[i++] = '-'; v = -v; }
  if (v == 0) { o[i++] = '0'; o[i] = 0; return; }
  int e = 0;
  while (v >= 10.0) { v /= 10.0; e++; }
  while (v < 1.0)   { v *= 10.0; e--; }
  v += 5e-10;
  if (v >= 10.0) { v /= 10.0; e++; }
  int dg[10], nd = 10;
  for (k = 0; k < 10; k++) { int d = (int)v; if (d > 9) d = 9; dg[k] = d; v = (v - d) * 10.0; }
  while (nd > 1 && dg[nd - 1] == 0) nd--;
  if (e >= -4 && e < 10) {
    if (e >= 0) {
      for (k = 0; k <= e; k++) o[i++] = (char)('0' + (k < nd ? dg[k] : 0));
      if (nd > e + 1) { o[i++] = '.'; for (k = e + 1; k < nd; k++) o[i++] = (char)('0' + dg[k]); }
    } else {
      o[i++] = '0'; o[i++] = '.';
      for (k = 0; k < -e - 1; k++) o[i++] = '0';
      for (k = 0; k < nd; k++) o[i++] = (char)('0' + dg[k]);
    }
    o[i] = 0;
  } else {
    o[i++] = (char)('0' + dg[0]);
    if (nd > 1) { o[i++] = '.'; for (k = 1; k < nd; k++) o[i++] = (char)('0' + dg[k]); }
    o[i++] = 'E'; o[i] = 0;
    ru_append_int(o, e);
  }
}
