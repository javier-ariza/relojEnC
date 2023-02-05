/*
 * reloj.h
 *
 *  Created on: 5 feb 2022
 *      Author: JAVIER
 */

#ifndef RELOJ_H_
#define RELOJ_H_

// Punto 1 v1

#include "systemConfig.h"


// Punto 2 v1 Completo

enum FSM_ESTADOS_RELOJ {
	WAIT_TIC
};

// Punto 3y4 v1

//En la Application Programming Interface, Interfaz de
//Programaci´on de Aplicaciones (API) del reloj del Ap´endice B se van
//enunciando las mas importantes
#define FLAG_ACTUALIZA_RELOJ 1
#define FLAG_TIME_ACTUALIZADO 2
#define MIN_DAY 1
#define MAX_MONTH 12
#define MIN_MONTH 1
#define MIN_YEAR 1970
#define MIN_HOUR 0
#define MAX_HOUR 23
#define MAX_MIN 59
#define MAX_SEG 59
#define TIME_FORMAT_12_H
#define TIME_FORMAT_24_H 24
#define PRECISION_RELOJ_MS 1000 // puede ser q no vaya aqui, es una etiqueta, hay que definirla en reloj.h, iniciarla con 1 segundo en milisegundos
#define DEFAULT_DAY 28
#define DEFAULT_MONTH 2
#define DEFAULT_YEAR 2020
#define DEFAULT_HOUR 0
#define DEFAULT_MIN 0
#define DEFAULT_SEC 0
#define DEFAULT_TIME_FORMAT 24

// Punto 5 v1
typedef struct {
	int dd;
	int MM;
	int yyyy ;
}TipoCalendario;

typedef struct {
	int hh;
	int mm;
	int ss;
	int formato ;
} TipoHora ;

typedef struct {
	int timestamp ;
	TipoHora hora ;
	TipoCalendario calendario ;
	tmr_t *tmrTic ;
}TipoReloj;

typedef struct  {
	int flags ;
} TipoRelojShared ;

// Punto 6 v1

extern fsm_trans_t g_fsmTransReloj [];


// Punto 7 v1
// estos arrays de declaran extern const int DIAS_MESES[2][MAX_MONTH];

//const int DIAS_MESES[2][MAX_MONTH]= {{31,29,31,30,31,30,31,31,30,31,30,31},{31,28,31,30,31,30,31,31,30,31,30,31}};

extern const int DIAS_MESES_BISIESTOS[MAX_MONTH];
extern const int DIAS_MESES_NO_BISIESTOS[MAX_MONTH];


// Punto 8 declarar prototipos de todas las funciones de la libreria reloj,figura 3.5

//FUNC. INICI VARS.

void ResetReloj ( TipoReloj* p_reloj);
int ConfiguraInicializaReloj ( TipoReloj* p_reloj );

// FUNCIONES PROPIAS

void ActualizaFecha ( TipoCalendario  *p_fecha );
void ActualizaHora ( TipoHora *p_hora );
int CalculaDiasMes (int month , int year);
int EsBisiesto (int year);
TipoRelojShared GetRelojSharedVar();
int SetHora (int horaInt , TipoHora *p_hora);
void SetRelojSharedVar (TipoRelojShared value);

// ENTRADA MAQUINA DE ESTADOS

int CompruebaTic (fsm_t* p_this);

// SALIDA MAQUINA DE ESTADOS

void ActualizaReloj (fsm_t* p_this);

// SUBRUTINAS DE AT. A LA INT.

void tmr_actualiza_reloj_isr (union sigval value);

#endif /* RELOJ_H_ */
