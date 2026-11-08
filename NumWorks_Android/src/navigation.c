#include "navigation.h"

static uint64_t s_prev = 0, s_next = 0;
static int s_rep = -1;

static int repeatable(int k) {
  return k == eadk_key_left || k == eadk_key_right || k == eadk_key_up ||
         k == eadk_key_down || k == eadk_key_backspace;
}

int nav_poll(void) {
  eadk_timing_msleep(20);
  uint64_t st = eadk_keyboard_scan();
  uint64_t now = eadk_timing_millis();
  s_prev &= st;
  uint64_t fresh = st & ~s_prev;
  if (fresh) {
    int k = 0;
    while (!((fresh >> k) & 1u)) k++;
    s_prev |= (1ULL << k);
    s_rep = k; s_next = now + 400;
    return k;
  }
  if (s_rep >= 0) {
    if (!((st >> s_rep) & 1u)) { s_rep = -1; return NAV_NONE; }
    if (g_settings.key_repeat && repeatable(s_rep) && now >= s_next) { s_next = now + 90; return s_rep; }
  }
  return NAV_NONE;
}
