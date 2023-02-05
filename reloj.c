/*
 * reloj.c
 *
 *  Created on: 11 de feb. de 2022
 *      Author: alumno
 */

#include "reloj.h"
#include "ent2004cfConfig.h"
#include "util.h"
#include <time.h>

// PASO 1 V1

fsm_trans_t g_fsmTransReloj[] = {
		{WAIT_TIC, CompruebaTic, WAIT_TIC, ActualizaReloj},
	    { -1, NULL, -1, NULL },
	    };

static TipoRelojShared g_relojSharedVars;
const int DIAS_MESES_BISIESTOS[] = {31,29,31,30,31,30,31,31,30,31,30,31};
const int DIAS_MESES_NO_BISIESTOS[] = {31,28,31,30,31,30,31,31,30,31,30,31};


// PASO 2y3

void ResetReloj ( TipoReloj* p_reloj) { // check

	TipoCalendario calendario;
	calendario.dd = DEFAULT_DAY;
	calendario.MM = DEFAULT_MONTH;
	calendario.yyyy = DEFAULT_YEAR;

	p_reloj->calendario = calendario;

	p_reloj->hora.hh = DEFAULT_HOUR;
	p_reloj->hora.mm = DEFAULT_MIN;
	p_reloj->hora.ss = DEFAULT_SEC;
	p_reloj->hora.formato = DEFAULT_TIME_FORMAT;
	p_reloj->timestamp = 0; // final sesion 2 punto 5

	piLock (RELOJ_KEY);
	g_relojSharedVars.flags = 0;
	piUnlock (RELOJ_KEY);
}


int ConfiguraInicializaReloj ( TipoReloj* p_reloj ) {

	ResetReloj(p_reloj);
	tmr_t* tmr = tmr_new (tmr_actualiza_reloj_isr);
	p_reloj->tmrTic = tmr ;
	tmr_startms_periodic(tmr, PRECISION_RELOJ_MS);
	return 0;
}

int CompruebaTic ( fsm_t* p_this ) {
    int result;
	piLock (RELOJ_KEY);
	result = (g_relojSharedVars.flags & FLAG_ACTUALIZA_RELOJ);
	piUnlock (RELOJ_KEY);

	return result;
}

void ActualizaReloj ( fsm_t* p_this ) {

	TipoReloj *p_miReloj = (TipoReloj*)(p_this->user_data);
	p_miReloj->timestamp++;
	ActualizaHora(&p_miReloj->hora);
	if ((p_miReloj->hora.hh == 00) && (p_miReloj->hora.mm == 00) && (p_miReloj->hora.ss == 00)){
    ActualizaFecha(&p_miReloj->calendario); // falta poner direccion
	}
	piLock (RELOJ_KEY);
	g_relojSharedVars.flags &= ~FLAG_ACTUALIZA_RELOJ;
	g_relojSharedVars.flags |= FLAG_TIME_ACTUALIZADO;
	piUnlock (RELOJ_KEY);
#if VERSION ==1
	piLock (RELOJ_KEY);
	printf("Son las: %02d:%02d:%02d del %02d/%02d/%d\n", p_miReloj->hora.hh, p_miReloj->hora.mm, p_miReloj->hora.ss,
			p_miReloj->calendario.dd, p_miReloj->calendario.MM, p_miReloj->calendario.yyyy);
	piUnlock (RELOJ_KEY);
#endif
}

void tmr_actualiza_reloj_isr ( union sigval value ) {

	piLock (RELOJ_KEY);
	g_relojSharedVars.flags |= FLAG_ACTUALIZA_RELOJ;
	piUnlock (RELOJ_KEY);
}

/*void ActualizaFecha ( TipoCalendario *p_fecha ){
   int diaSiguiente = p_fecha->dd + 1;
   int numDiasMes = CalculaDiasMes(p_fecha->MM,p_fecha->yyyy);
   int maximoDia = MAX(1, (numDiasMes+1) % diaSiguiente);
   p_fecha->dd = maximoDia;
   piLock (RELOJ_KEY);
   printf("Resultado dia, funcion fecha: %02d/%d\n",diaSiguiente,maximoDia) ;
   piUnlock (RELOJ_KEY);
   if (maximoDia == 1){
	   int mesSiguiente = p_fecha->MM + 1;
	   int maximoMes = MAX(1,(mesSiguiente %(MAX_MONTH+1)));
	   piLock (RELOJ_KEY);
	   printf("Resultado mes, funcion fecha: %02d/%d\n",mesSiguiente,maximoMes) ;
	   piUnlock (RELOJ_KEY);
	   p_fecha->MM = maximoMes;
   }
}*/

void ActualizaFecha ( TipoCalendario *p_fecha ){ // check

	int diaActual = p_fecha->dd;
	int diasMesActual = CalculaDiasMes((p_fecha->MM-1),p_fecha->yyyy);
	int moduloDias =   (diaActual + 1)%(diasMesActual + 1);
	int maxDias = MAX(1,moduloDias);
	p_fecha->dd = maxDias;

	piLock (STD_IO_LCD_BUFFER_KEY);
	printf("¡Cambio de fecha! diaActual, diasMes, y mes actual, funcion fecha: %02d/%d/%02d\n",diaActual,diasMesActual,p_fecha->MM) ;
	piUnlock (STD_IO_LCD_BUFFER_KEY);

	if (maxDias == 1){

	   int mesSiguiente = p_fecha->MM + 1;
	   int maximoMes = MAX(1,(mesSiguiente %(MAX_MONTH+1)));

	   piLock (RELOJ_KEY);
	   printf("Resultado mes, funcion fecha: %02d/%d\n",mesSiguiente,maximoMes) ;
	   piUnlock (RELOJ_KEY);

	   p_fecha->MM = maximoMes;

	   if (maximoMes == 1){

		   int anonuevo = p_fecha->yyyy + 1;
		   p_fecha->yyyy = anonuevo;
	   }
   }
}




void ActualizaHora ( TipoHora *p_hora ) {

	int nuevosSegundos = (p_hora->ss = (p_hora->ss + 1)%60);
	if (nuevosSegundos == 0){
		int nuevosMinutos = (p_hora->mm = (p_hora->mm + 1)%60);
		if ((nuevosMinutos == 0) && (nuevosSegundos == 0)){
			p_hora->hh = (p_hora->hh + 1)%TIME_FORMAT_24_H;//////
		}
	}
}
int CalculaDiasMes (int month, int year) {

	if (EsBisiesto(year)){
		return DIAS_MESES_BISIESTOS[month];
	}
	else {
		return DIAS_MESES_NO_BISIESTOS[month];
	}
}

int EsBisiesto (int year){
	if (year % 4 == 0){
		if (year%100 == 0){
			if(year%400 == 0){
				return 1;
			}
			else {return 0;}
		}
		else{return 1;}
		}
	else{ return 0;}
}

/*int SetHora (int horaInt,TipoHora *p_hora){

	int numero_digitos = 0;
	int aux = p_hora->hh;
	int auxHora = horaInt/100;
	int auxMin = horaInt - auxHora*100;

	if(horaInt < 0){
		return 10; // Error 10: Hora negativa
	}
	do {
		aux = aux/10;
		numero_digitos ++;
	} while (aux != 0);

	if(numero_digitos>4){
		return 11;  // Error 11: mayor numero de digitos que 4
	}

	if(horaInt > MAX_HOUR){
		auxHora = MAX_HOUR;
	}

	if(horaInt > MAX_MIN){
			auxMin = MAX_MIN;
		}

	p_hora->hh = auxHora;
	p_hora->mm = auxMin;
	p_hora->ss = DEFAULT_SEC;
	// p_hora->formato = DEFAULT_TIME_FORMAT;
	return 0;
}*/

int SetHora (int horaInt,TipoHora *p_hora){

	int numero_digitos = 0;
	int aux = horaInt;
	int auxHora = horaInt/100;
	int auxMin = horaInt - auxHora*100;

	if(horaInt < 0){
		return 10; // Error 10: Hora negativa
	}
	do {
		aux = aux/10;
		numero_digitos ++;
	} while (aux != 0);

	if(numero_digitos>4){
		return 11;  // Error 11: mayor numero de digitos que 4
	}

	if(auxHora > MAX_HOUR){
		auxHora = MAX_HOUR;
	}

	if(auxMin > MAX_MIN){
			auxMin = MAX_MIN;
		}

	p_hora->hh = auxHora;
	p_hora->mm = auxMin;
	p_hora->ss = DEFAULT_SEC;
	p_hora->formato = DEFAULT_TIME_FORMAT;
	return 0;
}

TipoRelojShared GetRelojSharedVar () {
	piLock (RELOJ_KEY);
	TipoRelojShared copiaGetRelojShareVar = g_relojSharedVars;
	piUnlock (RELOJ_KEY);
	return copiaGetRelojShareVar;
}


void SetRelojSharedVar (TipoRelojShared value)  {

	piLock (RELOJ_KEY);
	g_relojSharedVars = value ;
	piUnlock (RELOJ_KEY);
}

