#ifndef APPLICATIONS_H
#define APPLICATIONS_H
#include "ressources.h"

#define APP_COUNT 10
typedef struct {
  const char *label;   /* <= 9 caracteres (sous l'icone) */
  uint16_t color;
  void (*run)(void);   /* boucle propre a l'appli ; revient quand Retour/Home */
} app_t;

/* Pour ajouter une appli : ecrire sa fonction, l'ajouter dans g_apps,
   augmenter APP_COUNT et ajouter un glyphe dans ui_icon(). */
extern const app_t g_apps[APP_COUNT];

void app_calculator(void);
void app_settings(void);
void app_proba(void);
void app_suites(void);
void app_graph(void);
void app_functions(void);
void app_sheet(void);
void app_stats(void);
void app_regress(void);
void app_python(void);
#endif
