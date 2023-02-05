#include "coreWatch.h"
#include "reloj.h"


#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <wiringPiI2C.h>



//------------------------------------------------------
// DECLARACION DE VARIABLES GLOBALES
//------------------------------------------------------

TipoCoreWatch g_coreWatch;
static int g_flagsCoreWatch;

fsm_trans_t fsmTransCoreWatch[] = {

		{START, CompruebaSetupDone, STAND_BY, Start},
		{STAND_BY, CompruebaTimeActualizado, STAND_BY, ShowTime},


    	{STAND_BY, CompruebaSetCancelNewTime, SET_TIME, PrepareSetNewTime},
		{SET_TIME, CompruebaSetCancelNewTime, STAND_BY, CancelSetNewTime},
		{STAND_BY, CompruebaReset, STAND_BY, Reset},




     //   {STAND_BY, devuelveUno, SET_TIME, procesaUno},
		{SET_TIME, CompruebaNewTimeIsReady, STAND_BY, SetNewTime},
    	{SET_TIME, CompruebaDigitoPulsado, SET_TIME, ProcesaDigitoTime},




		{ -1, NULL, -1, NULL },
};

#if VERSION >= 3
fsm_trans_t fsmTransDeteccionComandos[] = {

		{WAIT_COMMAND, CompruebaTeclaPulsada, WAIT_COMMAND, ProcesaTeclaPulsada},
		{ -1, NULL, -1, NULL },
};
#endif








// FUNCIONES PROPIAS
//------------------------------------------------------
// Wait until next_activation (absolute time)
// Necesita de la función "delay" de WiringPi.

void DelayUntil(unsigned int next) {

	unsigned int now = millis();
	if (next > now) delay(next - now);
}

int EsNumero (char value ){

	if (value>=48 && value<=57) return value+1;
	return 0;
};

//------------------------------------------------------
// FUNCIONES THEARDS
//------------------------------------------------------
#if VERSION == 2
PI_THREAD ( ThreadExploraTecladoPC ) {
	int teclaPulsada;
	while (1) {
		delay (10) ; // WiringPi function : pauses program execution for at least 10 ms

		piLock(STD_IO_LCD_BUFFER_KEY);

		if( kbhit () ) {
			teclaPulsada = kbread ();
			int resEsNum = (EsNumero(teclaPulsada));
			// Logica ( diagrama de flujo ):
			if (teclaPulsada == TECLA_RESET) {
				piLock (SYSTEM_KEY);
				g_flagsCoreWatch |= FLAG_RESET;
				piUnlock (SYSTEM_KEY);


			}
			else if (teclaPulsada == TECLA_SET_CANCEL_TIME) {
				piLock (SYSTEM_KEY);
				g_flagsCoreWatch |= FLAG_SET_CANCEL_NEW_TIME;
				piUnlock (SYSTEM_KEY);
			}

			else if (resEsNum != 0) {
				piLock (SYSTEM_KEY);
				g_coreWatch.digitoPulsado = teclaPulsada - '0';  //*********
				g_flagsCoreWatch |= FLAG_DIGITO_PULSADO;
				piUnlock (SYSTEM_KEY);
			}
			else if (teclaPulsada == TECLA_EXIT) {
				piUnlock(STD_IO_LCD_BUFFER_KEY);
				piLock(STD_IO_LCD_BUFFER_KEY);
				printf ("\nSaliendo del programa...\n ");
				piUnlock(STD_IO_LCD_BUFFER_KEY);
				fflush(stdout);
				exit(0);
			}
			else if ((teclaPulsada != '\n') && (teclaPulsada != '\r')) {
				piUnlock(STD_IO_LCD_BUFFER_KEY);
				piLock(STD_IO_LCD_BUFFER_KEY);
                printf ("\nTecla desconocida \n ");
                piUnlock(STD_IO_LCD_BUFFER_KEY);
                fflush(stdout);
			}

			else {
				printf ("\nTecla desconocida \n ");
				fflush(stdout);
			}
		}
		piUnlock(STD_IO_LCD_BUFFER_KEY);
	}
}

#endif
//------------------------------------------------------
// FUNCIONES INICIALIZACION VARIABLES
//------------------------------------------------------
int ConfiguraInicializaSistema ( TipoCoreWatch* p_sistema ){
	piLock (SYSTEM_KEY);
	g_flagsCoreWatch = 0;
	piUnlock (SYSTEM_KEY);
	piLock (SYSTEM_KEY);
	p_sistema->tempTime = 0;
	p_sistema->digitosGuardados = 0;
	piUnlock (SYSTEM_KEY);
	int resultadoIni = ConfiguraInicializaReloj(&(p_sistema->reloj));

#if VERSION == 2
	int resultadoThre = piThreadCreate(ThreadExploraTecladoPC);
#endif

#if VERSION <= 3
	piLock (STD_IO_LCD_BUFFER_KEY);
	printf("Iniciando configuracion e inicializacion del reloj v3. \n");
	fflush(stdout);
	piUnlock (STD_IO_LCD_BUFFER_KEY);
#endif

	if ((resultadoIni != 0)) {
#if VERSION <= 3
		piLock (STD_IO_LCD_BUFFER_KEY);
		printf("Error reloj mal inicialziado. \n");
		fflush(stdout);
		piUnlock (STD_IO_LCD_BUFFER_KEY);
#endif
		return 22;
	} // Error reloj mal inicialziado


#if VERSION >= 3
	memcpy(p_sistema->teclado.filas, arrayFilas, sizeof(arrayFilas));
	memcpy (p_sistema->teclado.columnas, arrayColumnas, sizeof(arrayColumnas));
	int resultado = wiringPiSetupGpio();
	if (resultado < 0){
		return 32; //"error 32 error inicializacion driver"
	}
	ConfiguraInicializaTeclado(&(p_sistema->teclado));
#endif

#if VERSION == 2
	if ((resultadoThre != 0)) {
		piLock (STD_IO_LCD_BUFFER_KEY);
		printf("Error thread mal inicialziado. \n");
		fflush(stdout);
		piUnlock (STD_IO_LCD_BUFFER_KEY);
		return 13;
	} // Error thread
#endif


#if VERSION == 4

	int localAux = 0 ;
    localAux = lcdInit (2, 12, 8, GPIO_LCD_RS, GPIO_LCD_EN, GPIO_LCD_D0, GPIO_LCD_D1, GPIO_LCD_D2, GPIO_LCD_D3, GPIO_LCD_D4, GPIO_LCD_D5, GPIO_LCD_D6, GPIO_LCD_D7);
    p_sistema->lcdId = localAux;

    if(localAux == -1) {
    	return 40; // Error lcd mal inicializado
    }

#endif

#if VERSION <= 3
	piLock (STD_IO_LCD_BUFFER_KEY);
	printf("Configuracion e inicializacion correctas, del teclado tambien. \n");
	fflush(stdout);
	piUnlock (STD_IO_LCD_BUFFER_KEY);
#endif

	piLock (SYSTEM_KEY);
	g_flagsCoreWatch |= FLAG_SETUP_DONE;
	piUnlock (SYSTEM_KEY);
	return 0;
}

//------------------------------------------------------
// FUNCIONES DE ENTRADA O DE TRANSICION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------

int CompruebaDigitoPulsado ( fsm_t* p_this ) {

	int result =0;
	piLock (SYSTEM_KEY);
	result = (g_flagsCoreWatch & FLAG_DIGITO_PULSADO);
	piUnlock (SYSTEM_KEY);
	return result;
}

int CompruebaNewTimeIsReady ( fsm_t* p_this ){

	int result = 0 ;
	piLock (SYSTEM_KEY);
	result = (g_flagsCoreWatch & FLAG_NEW_TIME_IS_READY);
	piUnlock (SYSTEM_KEY);
	return result;
}

int CompruebaReset ( fsm_t* p_this ){

	int result = 0 ;
	piLock (SYSTEM_KEY);
	result = (g_flagsCoreWatch & FLAG_RESET);
	piUnlock (SYSTEM_KEY);
	return result;
}

int CompruebaTeclaSensor  ( fsm_t* p_this ) {

	int result = 0 ;
	piLock (SYSTEM_KEY);
	result = (g_flagsCoreWatch & FLAG_SENSOR);
	piUnlock (SYSTEM_KEY);
	return result;
}

int CompruebaTeclaGPS  ( fsm_t* p_this ) {

	int result = 0 ;
	piLock (SYSTEM_KEY);
	result = (g_flagsCoreWatch & FLAG_GPS);
	piUnlock (SYSTEM_KEY);
	return result;
}

int CompruebaSetCancelNewTime ( fsm_t* p_this ){

	int result = 0 ;

	piLock (SYSTEM_KEY);
	result = (g_flagsCoreWatch & FLAG_SET_CANCEL_NEW_TIME);
	piUnlock (SYSTEM_KEY);
	return result;
}

int CompruebaSetupDone ( fsm_t* p_this ){

	int result = 0 ;

	piLock (SYSTEM_KEY);
	result = (g_flagsCoreWatch & FLAG_SETUP_DONE);
	piUnlock (SYSTEM_KEY);
	return result;
}

int CompruebaTimeActualizado ( fsm_t* p_this ){

	TipoRelojShared aux = GetRelojSharedVar();
	int result = 0 ;
	result = (aux.flags & FLAG_TIME_ACTUALIZADO);
	return result;
}
#if VERSION >= 3
int CompruebaTeclaPulsada ( fsm_t* p_this ){

	TipoTecladoShared aux = GetTecladoSharedVar();
	int result = 0 ;
	result = (aux.flags & FLAG_TECLA_PULSADA);
	return result;
}
#endif

//------------------------------------------------------
//Funciones de salida de la maquina de estados
//------------------------------------------------------

void Start ( fsm_t* p_this ){

	piLock (SYSTEM_KEY);
	g_flagsCoreWatch &= ~FLAG_SETUP_DONE;
	piUnlock (SYSTEM_KEY);
}

void ShowTime ( fsm_t* p_this ){

  TipoCoreWatch* p_sistema = (TipoCoreWatch*)(p_this->user_data);

	TipoRelojShared aux = GetRelojSharedVar();
	piLock (SYSTEM_KEY);
    aux.flags  &= ~FLAG_TIME_ACTUALIZADO;
	piUnlock (SYSTEM_KEY);
	SetRelojSharedVar(aux);

#if VERSION <= 3
		piLock (STD_IO_LCD_BUFFER_KEY);
		printf("Son las: %02d:%02d:%02d del %02d/%02d/%d\n", p_sistema->reloj.hora.hh, p_sistema->reloj.hora.mm, p_sistema->reloj.hora.ss,
				p_sistema->reloj.calendario.dd, p_sistema->reloj.calendario.MM, p_sistema->reloj.calendario.yyyy);
		fflush(stdout);
		piUnlock (STD_IO_LCD_BUFFER_KEY);
#endif


#if VERSION == 4

		//a) Recupera el LCD del sistema (el identificador o handler lcdId) en una
		//variable local.
		int localAux;
		localAux = p_sistema->lcdId;
		//b) Limpia el LCD.
		lcdClear(localAux);

		//c) Imprime hora, minutos y segundos separados por ”:”: " %d: %d: %d" en la
		//primera fila.
		lcdPosition (localAux, 0, 0) ;
		piLock (STD_IO_LCD_BUFFER_KEY);
		lcdPrintf(localAux, "%02d:%02d:%02d", p_sistema->reloj.hora.hh, p_sistema->reloj.hora.mm, p_sistema->reloj.hora.ss);
		piUnlock (STD_IO_LCD_BUFFER_KEY);

		//d) Coloca el cursor en la primera columna de la segunda fila.
		//  x es la columna y 0
		// es el borde de la izquierda. y es la fila y 0 es la fila superior
		lcdPosition (localAux, 0, 1) ;
		//e) Imprime da, mes y ano separados por /”: " %d/ %d/ %d".
		//segunda fila.
		piLock (STD_IO_LCD_BUFFER_KEY);
		lcdPrintf(localAux, "%02d/%02d/%d", p_sistema->reloj.calendario.dd, p_sistema->reloj.calendario.MM, p_sistema->reloj.calendario.yyyy);
		piUnlock (STD_IO_LCD_BUFFER_KEY);
		//f ) Devuelve el cursor a la primera fila y primera columna.
		lcdPosition (localAux, 0, 0);

#endif


}

void Reset ( fsm_t* p_this ){

	TipoCoreWatch *p_sistema = (TipoCoreWatch*)(p_this->user_data);
	ResetReloj(&p_sistema->reloj);

	piLock (SYSTEM_KEY);
	g_flagsCoreWatch &= ~FLAG_RESET;
	piUnlock (SYSTEM_KEY);

#if VERSION <= 3
	piLock (STD_IO_LCD_BUFFER_KEY);
    printf("\n[RESET] Hora reiniciada\n");
	fflush(stdout);
	piUnlock (STD_IO_LCD_BUFFER_KEY);
#endif


#if VERSION == 4
	//a) Limpia el LCD.
	lcdClear (p_sistema->lcdId);

	//b) Coloca el cursor en la primera columna de la segunda fila.
	//  x es la columna y 0
	// es el borde de la izquierda. y es la fila y 0 es la fila superior
	lcdPosition (p_sistema->lcdId , 0, 1) ;

	//c) Imprime "RESET" y espera durante ESPERA_MENSAJE_MS milisegundos antes
	lcdPrintf (p_sistema->lcdId, "RESET") ;
	delay(ESPERA_MENSAJE_MS);
	//de limpiar de nuevo el LCD.

	lcdClear(p_sistema->lcdId);
#endif
}

void PrepareSetNewTime ( fsm_t* p_this ) {

	TipoCoreWatch *p_sistema = (TipoCoreWatch*)(p_this->user_data);
#if VERSION <= 3
	int formato = p_sistema->reloj.hora.formato; //para mejora 0 formato = 12, 1 formato = 24, fijarse en el printf de esta funcion // Solo en las versiones sin mejora
#endif

	piLock (SYSTEM_KEY);
	g_flagsCoreWatch &= ~FLAG_DIGITO_PULSADO;
	g_flagsCoreWatch &= ~FLAG_SET_CANCEL_NEW_TIME;
	piUnlock (SYSTEM_KEY);
#if VERSION <= 3
	piLock (STD_IO_LCD_BUFFER_KEY);
	printf ("\n[SET_TIME] Introduzca la nueva hora en formato (0-%d): \n ", formato);
	fflush(stdout);
	piUnlock (STD_IO_LCD_BUFFER_KEY);
#endif


#if VERSION >= 4

	//  Limpia el LCD.
	lcdClear (p_sistema->lcdId);

	 //Coloca el cursor en la
	//primera columna de la segunda fila.
	//  x es la columna y 0
	// es el borde de la izquierda. y es la fila y 0 es la fila superior
	lcdPosition ( p_sistema->lcdId , 0, 1) ;
    lcdPrintf(p_sistema->lcdId, "FORMAT: 0-%d",TIME_FORMAT_24_H);
	// Imprime "FORMAT: 0- %d". Donde %d imprimira el formato recogido.
   // lcdPrintf (int handle , const char * message , ...) ;


#endif


}

void CancelSetNewTime ( fsm_t* p_this ){

	TipoCoreWatch *p_sistema = (TipoCoreWatch*)(p_this->user_data);
	piLock (SYSTEM_KEY);
	p_sistema->tempTime = 0;
	p_sistema->digitosGuardados = 0;
	piUnlock (SYSTEM_KEY);
	piLock (SYSTEM_KEY);
	g_flagsCoreWatch &= ~FLAG_SET_CANCEL_NEW_TIME;
	piUnlock (SYSTEM_KEY);
#if VERSION <= 3
	piLock (STD_IO_LCD_BUFFER_KEY);
	printf ("[SET_TIME] Operacion cancelada \n");
	fflush(stdout);
	piUnlock (STD_IO_LCD_BUFFER_KEY);
#endif


#if VERSION == 4
	//a) Limpia el LCD.

	lcdClear (p_sistema->lcdId);
	//b) Coloca el cursor en la primera columna de la segunda fila.
	//  x es la columna y 0
	// es el borde de la izquierda. y es la fila y 0 es la fila superior
	lcdPosition (p_sistema->lcdId, 0, 1) ;

	//c) Imprime "CANCELADO" y espera durante ESPERA_MENSAJE_MS milisegundos antes
	lcdPrintf (p_sistema->lcdId,"CANCELADO") ;

	//de limpiar de nuevo el LCD.
	delay(ESPERA_MENSAJE_MS);
	lcdClear (p_sistema->lcdId);
#endif
}

void ProcesaDigitoTime ( fsm_t* p_this ){
	TipoCoreWatch *p_sistema = (TipoCoreWatch*)(p_this->user_data);

	int auxTemp = p_sistema->tempTime;
	int auxDigit = p_sistema->digitosGuardados;
	int ultimoDigito = p_sistema->digitoPulsado;
	piLock (SYSTEM_KEY);
	g_flagsCoreWatch &= ~FLAG_DIGITO_PULSADO;
	piUnlock (SYSTEM_KEY);



	switch (auxDigit){
	    case 0:
	    	if((p_sistema->reloj.hora.formato) == 12) {
	    				ultimoDigito = MIN(1, ultimoDigito);
	    	}else {
	    				ultimoDigito = MIN(2, ultimoDigito);
	    	}
	    	auxTemp = auxTemp*10 + ultimoDigito;
	        auxDigit ++;
	        break;

	    case 1:
	    	if ((p_sistema->reloj.hora.formato) == 12){
	    		if(auxTemp == 0){
	    			ultimoDigito = MAX(1, ultimoDigito);
	    		} else{
	    			ultimoDigito = MIN(2, ultimoDigito);
	    		}
    			auxTemp = auxTemp*10 + ultimoDigito;
    			auxDigit ++;
	    	}
	    	else{
	    		if(auxTemp == 2){
	    			ultimoDigito = MIN(3, ultimoDigito);
	    			auxTemp = auxTemp*10 + ultimoDigito;
	    			auxDigit ++;
	    		}
	    		else{
	    			auxTemp = auxTemp*10 + ultimoDigito;
	    			auxDigit ++;
	    		}
	    	}
	    	break;

	    case 2:
	    	auxTemp = auxTemp*10 + MIN(5, ultimoDigito);
	    	auxDigit ++;
	    	break;

	    default:
	    	auxTemp = auxTemp*10 + ultimoDigito;
	    	piLock (SYSTEM_KEY);
	    	g_flagsCoreWatch &= ~FLAG_DIGITO_PULSADO;
	    	piUnlock (SYSTEM_KEY);

	    	piLock (SYSTEM_KEY);
	        g_flagsCoreWatch |= FLAG_NEW_TIME_IS_READY;
	    	piUnlock (SYSTEM_KEY);
	}

	if ( auxDigit < 3) {
		if ( auxTemp > 2359) {
			auxTemp %= 10000;
			auxTemp = 100* MIN (( int) (auxTemp /100) , 23) + MIN(auxTemp %100 , 59) ;
		}
	}
	p_sistema->tempTime = auxTemp;
	p_sistema->digitosGuardados = auxDigit;

#if VERSION <= 3
	piLock (STD_IO_LCD_BUFFER_KEY);
	printf ("\n[SET_TIME] Nueva hora temporal %d\n", auxTemp);
	fflush(stdout);
	piUnlock (STD_IO_LCD_BUFFER_KEY);
#endif


#if VERSION == 4
	//a) Limpia la primera lnea del LCD (imprime espacios en blanco).
	lcdPosition(p_sistema->lcdId,0,0);
	lcdPrintf(p_sistema->lcdId,"            ");
	//b) Imprime en la primera lnea: "SET: %d" . Donde %d imprimira el
	//tiempo acumulado tempTime.
	lcdPosition(p_sistema->lcdId,0,0);
	lcdPrintf(p_sistema->lcdId,"SET: %d", p_sistema->tempTime);
#endif


}

void SetNewTime ( fsm_t * p_this ){

	TipoCoreWatch *p_sistema = (TipoCoreWatch*)(p_this->user_data);

	piLock (SYSTEM_KEY);
	g_flagsCoreWatch &= ~FLAG_NEW_TIME_IS_READY;
	piUnlock (SYSTEM_KEY);

	SetHora(p_sistema->tempTime, &p_sistema->reloj.hora);
	p_sistema->tempTime = 0;
	p_sistema->digitosGuardados = 0;
}


#if VERSION >= 3
void ProcesaTeclaPulsada ( fsm_t * p_this ){

	TipoCoreWatch *p_sistema = (TipoCoreWatch*)(p_this->user_data);

	TipoTecladoShared aux = GetTecladoSharedVar();
	piLock (KEYBOARD_KEY);
    aux.flags  &= ~FLAG_TECLA_PULSADA;
    piUnlock (KEYBOARD_KEY);
	SetTecladoSharedVar(aux);

	char teclaPulsada = aux.teclaDetectada;

	int resEsNum = (EsNumero(teclaPulsada));
	piLock (KEYBOARD_KEY);

				// Logica ( diagrama de flujo ):
		if (teclaPulsada == TECLA_RESET) {
			piLock (SYSTEM_KEY);
			g_flagsCoreWatch |= FLAG_RESET;
			piUnlock (SYSTEM_KEY);
		}
		else if (teclaPulsada == TECLA_SET_CANCEL_TIME) {
			piLock (SYSTEM_KEY);
			g_flagsCoreWatch |= FLAG_SET_CANCEL_NEW_TIME;
			piUnlock (SYSTEM_KEY);
		}

	    else if (resEsNum != 0) {
			piLock (SYSTEM_KEY);
			g_coreWatch.digitoPulsado = teclaPulsada - '0';  //*********
			g_flagsCoreWatch |= FLAG_DIGITO_PULSADO;
			piUnlock (SYSTEM_KEY);
		}

		else if (teclaPulsada == TECLA_EXIT) {
			piUnlock(STD_IO_LCD_BUFFER_KEY);
			piLock(STD_IO_LCD_BUFFER_KEY);
			lcdClear(p_sistema->lcdId);
			lcdPosition(p_sistema->lcdId,0,0);
			lcdPrintf (p_sistema->lcdId,"SALIENDO...");
			delay(ESPERA_MENSAJE_MS);
			lcdClear(p_sistema->lcdId);
			piUnlock(STD_IO_LCD_BUFFER_KEY);
			fflush(stdout);
     		exit(0);
		}

		else if ((teclaPulsada != '\n') && (teclaPulsada != '\r')) {
			piUnlock(STD_IO_LCD_BUFFER_KEY);
			piLock(STD_IO_LCD_BUFFER_KEY);
			lcdPosition(p_sistema->lcdId,0,0);
			lcdPrintf (p_sistema->lcdId,"DESCONOCIDA");
			delay(ESPERA_MENSAJE_MS);
			lcdPosition(p_sistema->lcdId,0,0);
			lcdPrintf (p_sistema->lcdId,"            ");
	        piUnlock(STD_IO_LCD_BUFFER_KEY);
	        fflush(stdout);
		}

	    else {
			printf ("\nTecla desconocida \n ");
			fflush(stdout);
		}

		piUnlock (KEYBOARD_KEY);
}

#endif


//------------------------------------------------------
// MAIN
//------------------------------------------------------

int main() {
unsigned int next;
#if VERSION <= 1
  TipoReloj relojPrueba;
  ConfiguraInicializaReloj(&relojPrueba);
  SetHora(2359, &relojPrueba.hora);
  fsm_t* fsmReloj = fsm_new(WAIT_TIC, g_fsmTransReloj, &(relojPrueba));

#endif


#if VERSION >= 2

  int iniCore = ConfiguraInicializaSistema(&g_coreWatch);
  if(iniCore!=0){
#if VERSION == 4
	  // FALTA HACER MOSTRAR POR PANTALLA EL ERROR DE INICIALIZAR
#endif

#if VERSION <= 3
	  piLock(STD_IO_LCD_BUFFER_KEY);
	  printf("ERROR: Configuracioninicialsistema(&g_corewatch) devuelve error: %d",iniCore);
	  piUnlock(STD_IO_LCD_BUFFER_KEY);
	  fflush(stdout);
#endif
 	  exit(0);
   }
 fsm_t* fsmReloj = fsm_new(WAIT_TIC, g_fsmTransReloj, &(g_coreWatch.reloj));
 fsm_t* fsmCoreWatch = fsm_new(START, fsmTransCoreWatch, &(g_coreWatch));


#endif


#if VERSION >= 3 // puede q falte lanzar las maquinas etc



 fsm_t* deteccionComandosFSM = fsm_new(WAIT_COMMAND, fsmTransDeteccionComandos, &(g_coreWatch));
 fsm_t* tecladoFSM = fsm_new(TECLADO_ESPERA_COLUMNA, g_fsmTransExcitacionColumnas, &(g_coreWatch.teclado));
#if VERSION <= 3
  piLock(STD_IO_LCD_BUFFER_KEY);
  printf("Configuracion teclado lanzada\n");
  piUnlock(STD_IO_LCD_BUFFER_KEY);
#endif
#endif







 next = millis();
  while (1) {
#if VERSION >= 3
	  fsm_fire(deteccionComandosFSM);
	  fsm_fire(tecladoFSM);
#endif

#if VERSION == 1
	  fsm_fire(fsmReloj);
#endif
#if VERSION >= 2
	  fsm_fire(fsmReloj);
	  fsm_fire(fsmCoreWatch);
#endif

	next += CLK_MS;
	DelayUntil(next);
}




// tmr_destroy((tmr_t*)(g_coreWatch->reloj.tmrTic->user_data));
#if VERSION == 1
  fsm_destroy (fsmReloj);
#endif
#if VERSION >= 2
   fsm_destroy (fsmCoreWatch);
  fsm_destroy (fsmReloj);

#if VERSION >= 3
  fsm_destroy (deteccionComandosFSM);
  fsm_destroy (tecladoFSM);
#endif
#endif
}
