#ifndef NAVIGATION_H
#define NAVIGATION_H
#include "ressources.h"
#define NAV_NONE (-1)
/* Attend ~20 ms (pas de boucle active) puis renvoie l'index de la touche
   nouvellement pressee (eadk_key_*) ou NAV_NONE. Repetition auto des fleches. */
int nav_poll(void);
#endif
