#include "applications.h"
#include "interface.h"
#include "navigation.h"

/* ================= Aides communes ================= */
static int is_back(int k) { return k == eadk_key_back || k == eadk_key_home; }
static int is_ok(int k) { return k == eadk_key_ok || k == eadk_key_exe; }

static void screen_begin(const char *title) {
  ui_fill(0, 0, SCREEN_W, SCREEN_H, res_theme()->bg);
  ui_titlebar(title);
}

static void list_row(int y, const char *label, const char *val, int sel, int arrows) {
  const theme_t *t = res_theme();
  uint16_t bg = sel ? res_accent() : t->card, fg = sel ? WHITE : t->text;
  ui_round(8, y, 304, 22, 8, bg);
  ui_text(label, 16, y + 4, 0, fg, bg);
  if (val) {
    char b[40]; b[0] = 0;
    if (arrows && sel) ru_append(b, "< ");
    ru_append(b, val);
    if (arrows && sel) ru_append(b, " >");
    ui_text(b, 304 - ru_strlen(b) * 7, y + 4, 0, fg, bg);
  }
}

static void hint(int y, const char *s) { ui_text(s, 12, y, 0, res_theme()->sub, res_theme()->bg); }

static void wait_back(void) {
  for (;;) { int k = nav_poll(); if (k != NAV_NONE && (is_back(k) || is_ok(k))) return; }
}

static void info_screen(const char *title, const char *const *lines, int n) {
  const theme_t *t = res_theme();
  screen_begin(title);
  for (int i = 0; i < n; i++) ui_text(lines[i], 12, 34 + i * 20, 0, i == 0 ? res_accent() : t->text, t->bg);
  hint(216, "OK / Retour : fermer");
  wait_back();
}

static void placeholder(const char *title, const char *l1, const char *l2) {
  const theme_t *t = res_theme();
  screen_begin(title);
  ui_round(8, 34, 304, 100, 12, t->card);
  ui_text("MODULE A DEVELOPPER", 20, 44, 1, res_accent(), t->card);
  ui_text(l1, 20, 74, 0, t->text, t->card);
  ui_text(l2, 20, 92, 0, t->text, t->card);
  ui_text("Non implemente dans cette version.", 20, 112, 0, t->sub, t->card);
  hint(146, "L'appli Epsilon d'origine ne peut pas etre");
  hint(162, "lancee depuis une appli EADK : ce module");
  hint(178, "doit etre reecrit ici (voir README).");
  hint(216, "OK / Retour : accueil");
  wait_back();
}

/* ================= Calculatrice (reelle) ================= */
static const char *ps; static int perr; static double pans;
static double p_expr(void);
static double p_atom(void) {
  if (*ps == '(') { ps++; double v = p_expr(); if (*ps == ')') ps++; else perr = 1; return v; }
  if (*ps == 'A') { ps++; return pans; }
  if ((*ps >= '0' && *ps <= '9') || *ps == '.') {
    double v = 0;
    while (*ps >= '0' && *ps <= '9') { v = v * 10 + (*ps - '0'); ps++; }
    if (*ps == '.') { ps++; double f = 0.1; while (*ps >= '0' && *ps <= '9') { v += (*ps - '0') * f; f *= 0.1; ps++; } }
    return v;
  }
  perr = 1; return 0;
}
static double p_unary(void);
static double p_pow(void) {
  double b = p_atom();
  if (*ps == '^') {
    ps++;
    double e = p_unary(); int n = (int)e;
    if ((double)n != e || n > 300 || n < -300) { perr = 1; return 0; }
    int an = n < 0 ? -n : n; double r = 1;
    for (int i = 0; i < an; i++) r *= b;
    if (n < 0) { if (r == 0) { perr = 1; return 0; } r = 1.0 / r; }
    return r;
  }
  return b;
}
static double p_unary(void) {
  if (*ps == '-') { ps++; return -p_unary(); }
  if (*ps == '+') { ps++; return p_unary(); }
  return p_pow();
}
static double p_term(void) {
  double v = p_unary();
  while (*ps == '*' || *ps == '/') {
    char o = *ps++; double r = p_unary();
    if (o == '*') v *= r; else { if (r == 0) { perr = 1; return 0; } v /= r; }
  }
  return v;
}
static double p_expr(void) {
  double v = p_term();
  while (*ps == '+' || *ps == '-') { char o = *ps++; double r = p_term(); if (o == '+') v += r; else v -= r; }
  return v;
}
static int calc_eval(const char *s, double ans, double *out) {
  ps = s; perr = 0; pans = ans;
  double v = p_expr();
  if (*ps) perr = 1;
  if (perr) return 0;
  *out = v; return 1;
}

static const struct { int key; char ch; } calc_keys[] = {
  {eadk_key_zero,'0'},{eadk_key_one,'1'},{eadk_key_two,'2'},{eadk_key_three,'3'},{eadk_key_four,'4'},
  {eadk_key_five,'5'},{eadk_key_six,'6'},{eadk_key_seven,'7'},{eadk_key_eight,'8'},{eadk_key_nine,'9'},
  {eadk_key_dot,'.'},{eadk_key_plus,'+'},{eadk_key_minus,'-'},{eadk_key_multiplication,'*'},
  {eadk_key_division,'/'},{eadk_key_leftparenthesis,'('},{eadk_key_rightparenthesis,')'},
  {eadk_key_power,'^'},{eadk_key_ans,'A'},
};

void app_calculator(void) {
  char expr[48], res[32]; int len = 0, dirty = 1; double ans = 0;
  expr[0] = 0; res[0] = 0;
  for (;;) {
    if (dirty) {
      const theme_t *t = res_theme();
      screen_begin("Calculatrice");
      ui_round(8, 30, 304, 50, 10, t->card);
      const char *s = expr; if (len > 28) s = expr + len - 28;
      ui_text(s, 16, 46, 1, t->text, t->card);
      ui_round(8, 88, 304, 44, 10, res_accent());
      ui_text(res, 304 - ru_strlen(res) * 10, 101, 1, WHITE, res_accent());
      hint(146, "OK/EXE : calculer    VAR : tout effacer");
      hint(162, "Suppr : efface 1 car.  Ans : dernier res.");
      hint(178, "0-9 . + - x / ( ) ^   exposant entier");
      hint(216, "Retour : accueil");
      dirty = 0;
    }
    int k = nav_poll();
    if (k == NAV_NONE) continue;
    if (is_back(k)) return;
    dirty = 1;
    if (k == eadk_key_backspace) { if (len) expr[--len] = 0; res[0] = 0; continue; }
    if (k == eadk_key_var) { len = 0; expr[0] = 0; res[0] = 0; continue; }
    if (is_ok(k)) {
      if (len) {
        double v;
        if (calc_eval(expr, ans, &v)) { ans = v; ru_fmt_double(v, res); }
        else { res[0] = 0; ru_append(res, "Erreur"); }
      }
      continue;
    }
    for (unsigned i = 0; i < sizeof(calc_keys) / sizeof(calc_keys[0]); i++)
      if (calc_keys[i].key == k && len < 45) { expr[len++] = calc_keys[i].ch; expr[len] = 0; res[0] = 0; }
  }
}

/* ================= Probabilites (reelle) ================= */
static double binom_c(int n, int k) {
  if (k < 0 || k > n) return 0;
  if (k > n - k) k = n - k;
  double c = 1;
  for (int i = 0; i < k; i++) c = c * (n - i) / (i + 1);
  return c;
}
static double ipow(double b, int e) { double r = 1; for (int i = 0; i < e; i++) r *= b; return r; }

static void res_line(int y, const char *label, double v) {
  const theme_t *t = res_theme();
  char b[48]; b[0] = 0;
  ru_append(b, label);
  char n[32]; ru_fmt_double(v, n); ru_append(b, n);
  ui_text(b, 20, y, 0, t->text, t->card);
}

void app_proba(void) {
  int n = 10, k = 3, p20 = 10, f = 0, dirty = 1;
  for (;;) {
    if (dirty) {
      const theme_t *t = res_theme();
      screen_begin("Probabilites");
      char b[16];
      b[0] = 0; ru_append_int(b, n);  list_row(30, "n (essais)", b, f == 0, 1);
      b[0] = 0; ru_append_int(b, k);  list_row(54, "k (succes)", b, f == 1, 1);
      { double p = p20 / 20.0; ru_fmt_double(p, b); list_row(78, "p (proba)", b, f == 2, 1); }
      ui_round(8, 108, 304, 100, 10, t->card);
      double q = 1 - p20 / 20.0, p = p20 / 20.0, cum = 0, fact = 1, arr = 1;
      for (int i = 2; i <= n; i++) fact *= i;
      for (int i = 0; i < k; i++) arr *= (n - i);
      for (int i = 0; i <= k; i++) cum += binom_c(n, i) * ipow(p, i) * ipow(q, n - i);
      res_line(116, "n!       = ", fact);
      res_line(134, "C(n,k)   = ", binom_c(n, k));
      res_line(152, "A(n,k)   = ", arr);
      res_line(170, "P(X=k)   = ", binom_c(n, k) * ipow(p, k) * ipow(q, n - k));
      res_line(188, "P(X<=k)  = ", cum);
      hint(216, "Haut/Bas: champ  Gauche/Droite: valeur");
      dirty = 0;
    }
    int kk = nav_poll();
    if (kk == NAV_NONE) continue;
    if (is_back(kk)) return;
    dirty = 1;
    if (kk == eadk_key_up) f = (f + 2) % 3;
    else if (kk == eadk_key_down) f = (f + 1) % 3;
    else {
      int dv = kk == eadk_key_right ? 1 : (kk == eadk_key_left ? -1 : 0);
      if (f == 0) { n += dv; if (n < 0) n = 0; if (n > 60) n = 60; if (k > n) k = n; }
      if (f == 1) { k += dv; if (k < 0) k = 0; if (k > n) k = n; }
      if (f == 2) { p20 += dv; if (p20 < 0) p20 = 0; if (p20 > 20) p20 = 20; }
    }
  }
}

/* ================= Suites (reelle) ================= */
void app_suites(void) {
  int geo = 0, u0 = 1, r2 = 2, n = 10, f = 0, dirty = 1;
  for (;;) {
    if (dirty) {
      const theme_t *t = res_theme();
      screen_begin("Suites");
      char b[32];
      list_row(30, "Type", geo ? "Geometrique" : "Arithmetique", f == 0, 1);
      b[0] = 0; ru_append_int(b, u0); list_row(54, "u0", b, f == 1, 1);
      ru_fmt_double(r2 / 2.0, b); list_row(78, geo ? "raison q" : "raison r", b, f == 2, 1);
      b[0] = 0; ru_append_int(b, n); list_row(102, "n", b, f == 3, 1);
      ui_round(8, 132, 304, 74, 10, t->card);
      double u = u0, s = u0, rr = r2 / 2.0;
      for (int i = 0; i < n; i++) { if (geo) u *= rr; else u += rr; s += u; }
      ui_text(geo ? "u(n+1) = u(n) x q" : "u(n+1) = u(n) + r", 20, 140, 0, t->sub, t->card);
      res_line(160, "u(n)      = ", u);
      res_line(180, "u0+...+un = ", s);
      hint(216, "Haut/Bas: champ  Gauche/Droite: valeur");
      dirty = 0;
    }
    int k = nav_poll();
    if (k == NAV_NONE) continue;
    if (is_back(k)) return;
    dirty = 1;
    if (k == eadk_key_up) f = (f + 3) % 4;
    else if (k == eadk_key_down) f = (f + 1) % 4;
    else {
      int dv = k == eadk_key_right ? 1 : (k == eadk_key_left ? -1 : 0);
      if (f == 0 && dv) geo = !geo;
      if (f == 1) { u0 += dv; if (u0 < -99) u0 = -99; if (u0 > 99) u0 = 99; }
      if (f == 2) { r2 += dv; if (r2 < -40) r2 = -40; if (r2 > 40) r2 = 40; }
      if (f == 3) { n += dv; if (n < 0) n = 0; if (n > 60) n = 60; }
    }
  }
}

/* ================= Parametres (reels) ================= */
static void time_screen(void) {
  nwclock_t c; clock_get(&c);
  int f = 0, dirty = 1;
  for (;;) {
    if (dirty) {
      screen_begin("Heure et date");
      char b[8];
      b[0] = 0; ru_append_int(b, c.h);  list_row(30, "Heure", b, f == 0, 1);
      b[0] = 0; ru_append_int(b, c.m);  list_row(54, "Minute", b, f == 1, 1);
      b[0] = 0; ru_append_int(b, c.d);  list_row(78, "Jour", b, f == 2, 1);
      b[0] = 0; ru_append_int(b, c.mo); list_row(102, "Mois", b, f == 3, 1);
      b[0] = 0; ru_append_int(b, c.y);  list_row(126, "Annee", b, f == 4, 1);
      hint(166, "Pas d'horloge materielle accessible :");
      hint(182, "l'heure se regle a chaque lancement.");
      hint(216, "OK : valider    Retour : annuler");
      dirty = 0;
    }
    int k = nav_poll();
    if (k == NAV_NONE) continue;
    if (is_back(k)) return;
    dirty = 1;
    if (is_ok(k)) { clock_set(c.h, c.m, c.d, c.mo, c.y); return; }
    if (k == eadk_key_up) f = (f + 4) % 5;
    else if (k == eadk_key_down) f = (f + 1) % 5;
    else {
      int dv = k == eadk_key_right ? 1 : (k == eadk_key_left ? -1 : 0);
      if (f == 0) c.h = (c.h + dv + 24) % 24;
      if (f == 1) c.m = (c.m + dv + 60) % 60;
      if (f == 2) { c.d += dv; int dim = clock_days_in_month(c.mo, c.y); if (c.d < 1) c.d = dim; if (c.d > dim) c.d = 1; }
      if (f == 3) c.mo = (c.mo - 1 + dv + 12) % 12 + 1;
      if (f == 4) { c.y += dv; if (c.y < 2000) c.y = 2099; if (c.y > 2099) c.y = 2000; }
      int dim = clock_days_in_month(c.mo, c.y); if (c.d > dim) c.d = dim;
    }
  }
}

void app_settings(void) {
  const int N = 7;
  int sel = 0, dirty = 1;
  for (;;) {
    if (dirty) {
      screen_begin("Parametres");
      list_row(30,  "Theme", res_theme()->name, sel == 0, 1);
      list_row(54,  "Couleur d'accent", res_accent_name(), sel == 1, 1);
      list_row(78,  "Barre de navigation", g_settings.navbar ? "Oui" : "Non", sel == 2, 1);
      list_row(102, "Repetition des touches", g_settings.key_repeat ? "Oui" : "Non", sel == 3, 1);
      list_row(126, "Regler heure et date", ">", sel == 4, 0);
      list_row(150, "A propos de l'application", ">", sel == 5, 0);
      list_row(174, "Infos calculatrice", ">", sel == 6, 0);
      hint(204, "Haut/Bas: choisir  Gauche/Droite: modifier");
      hint(220, "OK: ouvrir      Retour: accueil");
      dirty = 0;
    }
    int k = nav_poll();
    if (k == NAV_NONE) continue;
    if (is_back(k)) return;
    dirty = 1;
    if (k == eadk_key_up) { sel = (sel + N - 1) % N; continue; }
    if (k == eadk_key_down) { sel = (sel + 1) % N; continue; }
    int dv = k == eadk_key_right ? 1 : (k == eadk_key_left ? -1 : 0);
    int ok = is_ok(k);
    if (!dv && ok) dv = 1;
    if (!dv) continue;
    if (sel == 0) g_settings.theme = (g_settings.theme + dv + res_theme_count()) % res_theme_count();
    else if (sel == 1) g_settings.accent = (g_settings.accent + dv + res_accent_count()) % res_accent_count();
    else if (sel == 2) g_settings.navbar = !g_settings.navbar;
    else if (sel == 3) g_settings.key_repeat = !g_settings.key_repeat;
    else if (sel == 4 && ok) time_screen();
    else if (sel == 5 && ok) {
      static const char *const l[] = {
        "NumWorks Android OS", "Version " NWOS_VERSION, "Appli EADK independante (C)",
        "Interface inspiree d'Android / One UI", "Reglages non sauvegardes : l'EADK n'a",
        "pas de stockage persistant."};
      info_screen("A propos", l, 6);
    } else if (sel == 6 && ok) {
      static const char *const l[] = {
        "NumWorks N0120", "Epsilon cible : 26.4.0", "Ecran : 320 x 240, RGB565",
        "Aucun ecran tactile : touches seules", "Apps Epsilon d'origine : non lancables"};
      info_screen("Calculatrice", l, 5);
    }
  }
}

/* ================= Modules a developper ================= */
void app_graph(void)     { placeholder("Calculatrice graphique", "Trace de courbes y = f(x).", "A coder : parseur + trace sur 320x240."); }
void app_functions(void) { placeholder("Fonctions", "Definition et etude de fonctions.", "A coder : liste f1..fn, valeurs, zeros."); }
void app_sheet(void)     { placeholder("Tableur", "Grille de cellules et formules.", "A coder : grille, saisie, formules."); }
void app_stats(void)     { placeholder("Statistiques", "Series, moyenne, ecart-type.", "A coder : saisie de listes, indicateurs."); }
void app_regress(void)   { placeholder("Regressions", "Ajustement lineaire et autres.", "A coder : moindres carres sur les listes."); }
void app_python(void)    { placeholder("Python", "Interpreteur Python.", "Impossible via l'EADK : utiliser l'app Epsilon."); }

/* ================= Table des applications ================= */
const app_t g_apps[APP_COUNT] = {
  {"Calcul.",   RGB565(255,149,0),  app_calculator},
  {"Graphe",    RGB565(52,120,246), app_graph},
  {"Fonctions", RGB565(150,90,230), app_functions},
  {"Tableur",   RGB565(40,170,90),  app_sheet},
  {"Stats",     RGB565(0,165,180),  app_stats},
  {"Regress.",  RGB565(235,70,120), app_regress},
  {"Suites",    RGB565(220,160,0),  app_suites},
  {"Proba",     RGB565(225,70,60),  app_proba},
  {"Python",    RGB565(55,118,171), app_python},
  {"Reglages",  RGB565(105,115,135),app_settings},
};
