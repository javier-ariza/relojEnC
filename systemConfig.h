#ifndef _SYSTEMCONFIG_H_
#define _SYSTEMCONFIG_H_
//------------------------------------------------------
// INCLUDES
// Del sistema
#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>

// Propios
#include "ent2004cfConfig.h"  // Entorno emulado o de laboratorio. GPIOs.
#include "fsm.h"
#include "tmr.h"
#include "reloj.h"
#include "util.h"
#include "teclado_TL04.h"
#include <lcd.h>
#if ENTORNO == ENTORNO_LABORATORIO
#include "kbhit.h" // Para poder detectar teclas pulsadas sin bloqueo y leer las teclas pulsadas
#include <wiringPi.h>

#elif ENTORNO == ENTORNO_EMULADO
#include "pseudoWiringPi.h"
#endif

// DEFINES Y ENUMS
#define VERSION	4  // Version del sistema (1, 2, 3, 4...)
#define CLK_MS	10  // Actualizacion de las maquinas de estado (ms)


// DECLARACIÓN DE VARIABLES


// DEFINICIÓN DE VARIABLES


#endif /* SYSTEMCONFIG_H_ */
