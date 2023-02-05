#ifndef COREWATCH_H_
#define COREWATCH_H_

// INCLUDES
// Propios:
#include "systemConfig.h"     // Sistema: includes, entrenadora (GPIOs, MUTEXes y entorno), setup de perifericos y otros otros.
// DEFINES Y ENUMS

enum FSM_ESTADOS_SISTEMA{
	START,
	STAND_BY,
	SET_TIME
};

enum FSM_DETECCION_COMANDOS{
	WAIT_COMMAND
};


// FLAGS FSM DEL SISTEMA CORE WATCH
#define FLAG_SETUP_DONE 4
#define FLAG_RESET 8
#define FLAG_SET_CANCEL_NEW_TIME 16
#define FLAG_NEW_TIME_IS_READY 32
#define FLAG_DIGITO_PULSADO 64
#define FLAG_SENSOR 128
#define FLAG_GPS 256
//#define FLAG_TECLA_PULSADA 128
#define TECLA_RESET 'F'
#define TECLA_EXIT 'B'
#define TECLA_SET_CANCEL_TIME 'E'
#define TECLA_SENSOR 'S'
#define TECLA_GPS 'L'
#define ESPERA_MENSAJE_MS 500
// DECLARACIÓN ESTRUCTURAS

typedef struct {
	TipoReloj reloj ;
	TipoTeclado teclado ;
	int lcdId ;
	int tempTime ;
	int digitosGuardados ;
	int digitoPulsado ;

} TipoCoreWatch ;

// DECLARACIÓN VARIABLES

extern fsm_trans_t fsmTransCoreWatch[] ;
extern int32_t t_fine;
extern float t ; // C
extern float p ; // hPa
extern float h;       // %
extern float latitud;
extern float longitud;


// DEFINICIÓN VARIABLES


//------------------------------------------------------
// FUNCIONES DE INICIALIZACION DE LAS VARIABLES
//------------------------------------------------------
int devuelveUno();
void procesaUno();
//------------------------------------------------------
// FUNCIONES PROPIAS
//------------------------------------------------------
void DelayUntil(unsigned int next);
int ConfiguraInicializaSistema ();
void configuracionInicializacionSensor ();
int EsNumero ( char value ) ;
//------------------------------------------------------
// FUNCIONES DE ENTRADA O DE TRANSICION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------
int CompruebaDigitoPulsado ( fsm_t* p_this ) ;
int CompruebaNewTimeIsReady ( fsm_t* p_this ) ;
int CompruebaReset ( fsm_t* p_this ) ;
int CompruebaTeclaSensor  ( fsm_t* p_this ) ;
int CompruebaTeclaGPS  ( fsm_t* p_this );
int CompruebaSetCancelNewTime ( fsm_t* p_this ) ;
int CompruebaSetupDone ( fsm_t* p_this ) ;
int CompruebaTeclaPulsada ( fsm_t* p_this ) ;
int CompruebaTimeActualizado ( fsm_t* p_this ) ;
//------------------------------------------------------
// FUNCIONES DE SALIDA O DE ACCION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------
void CancelSetNewTime ( fsm_t* p_this );
void PrepareSetNewTime ( fsm_t* p_this ) ;
void ProcesaDigitoTime ( fsm_t* p_this ) ;
void ProcesaTeclaPulsada ( fsm_t* p_this ) ;
void Reset ( fsm_t* p_this ) ;
void muestraMedidaSensor (fsm_t* p_this );
void muestraMedidaGPS (fsm_t* p_this );
void SetNewTime ( fsm_t* p_this ) ;
void ShowTime ( fsm_t* p_this ) ;
void Start ( fsm_t* p_this ) ;
//------------------------------------------------------
// SUBRUTINAS DE ATENCION A LAS INTERRUPCIONES
//------------------------------------------------------


//------------------------------------------------------
// FUNCIONES LIGADAS A THREADS ADICIONALES
//------------------------------------------------------
#if VERSION == 2
PI_THREAD ( ThreadExploraTecladoPC ) ;
# endif
#endif /* EAGENDA_H */
