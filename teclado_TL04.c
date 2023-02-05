#include "teclado_TL04.h"

const char tecladoTL04[NUM_FILAS_TECLADO][NUM_COLUMNAS_TECLADO] = {
		{'1', '2', '3', 'C'},
		{'4', '5', '6', 'D'},
		{'7', '8', '9', 'E'},
		{'A', '0', 'B', 'F'}
};

// Maquina de estados: lista de transiciones
// {EstadoOrigen, CondicionDeDisparo, EstadoFinal, AccionesSiTransicion }
fsm_trans_t g_fsmTransExcitacionColumnas[] = {
		{ TECLADO_ESPERA_COLUMNA, CompruebaTimeoutColumna, TECLADO_ESPERA_COLUMNA, TecladoExcitaColumna },
		{-1, NULL, -1, NULL },
};

static TipoTecladoShared g_tecladoSharedVars;



const int arrayColumnas[] = {GPIO_KEYBOARD_COL_1,GPIO_KEYBOARD_COL_2,GPIO_KEYBOARD_COL_3,GPIO_KEYBOARD_COL_4};
const int arrayFilas[] = {GPIO_KEYBOARD_ROW_1,GPIO_KEYBOARD_ROW_2,GPIO_KEYBOARD_ROW_3,GPIO_KEYBOARD_ROW_4};
static void (* array_row_p[4]) ()={teclado_fila_1_isr,teclado_fila_2_isr,teclado_fila_3_isr,teclado_fila_4_isr};

//------------------------------------------------------
// FUCNIONES DE INICIALIZACION DE LAS VARIABLES ESPECIFICAS
//------------------------------------------------------
void ConfiguraInicializaTeclado(TipoTeclado *p_teclado) {
	// A completar por el alumno...

	// Inicializacion de elementos de la variable global de tipo TipoTecladoShared:
	// 1. Valores iniciales de todos los "debounceTime"

		piLock (KEYBOARD_KEY);
		g_tecladoSharedVars.debounceTime[FILA_1] = 0;
		g_tecladoSharedVars.debounceTime[FILA_2] = 0;
		g_tecladoSharedVars.debounceTime[FILA_3] = 0;
		g_tecladoSharedVars.debounceTime[FILA_4] = 0;
		piUnlock (KEYBOARD_KEY);

 // tiempo de guarda anti-rebotes es ahora 0
	// 2. Valores iniciales de todos "columnaActual", "teclaDetectada" y "flags"
	piLock (KEYBOARD_KEY);
	g_tecladoSharedVars.columnaActual = COLUMNA_1;
	piUnlock (KEYBOARD_KEY);
	piLock (KEYBOARD_KEY);
	g_tecladoSharedVars.teclaDetectada = 'Z';
	piUnlock (KEYBOARD_KEY);
	piLock (KEYBOARD_KEY);
	g_tecladoSharedVars.flags = 0;
	piUnlock (KEYBOARD_KEY);

	// Inicializacion de elementos de la estructura TipoTeclado:


	// preguntar sobre iniciañizacion timer

	// Inicializacion del HW:
	// 1. Configura GPIOs de las columnas:
	// 	  (i) Configura los pines y (ii) da valores a la salida
	pinMode(GPIO_KEYBOARD_COL_1, OUTPUT);
	digitalWrite(GPIO_KEYBOARD_COL_1, HIGH);

	pinMode(GPIO_KEYBOARD_COL_2, OUTPUT);
	digitalWrite(GPIO_KEYBOARD_COL_2, LOW);

	pinMode(GPIO_KEYBOARD_COL_3, OUTPUT);
	digitalWrite(GPIO_KEYBOARD_COL_3, LOW);

	pinMode(GPIO_KEYBOARD_COL_4, OUTPUT);
	digitalWrite(GPIO_KEYBOARD_COL_4, LOW);



	// 2. Configura GPIOs de las filas:
	// 	  (i) Configura los pines y (ii) asigna ISRs (y su polaridad)
	//
	int j=0;
	for (j=0; j<4; j++){
		pinMode(arrayFilas[j], INPUT);
		pullUpDnControl (arrayFilas[j], PUD_DOWN ); // todas las filas inicialmente a pull down
		wiringPiISR(arrayFilas[j], INT_EDGE_RISING, array_row_p[j]);
		}
	// array_row_p
		//declaración de un array de rutinas de interrupción que se ejecutan
		// dependiendo del pin en el que se produce un cambio de nivel
		// detectan flanco de subida




	// Inicializacion del temporizador:
	// 3. Crear y asignar temporizador de excitacion de columnas
	tmr_t* timer = tmr_new (timer_duracion_columna_isr);
	p_teclado->tmr_duracion_columna =  timer;

	// 4. Lanzar temporizador
	tmr_startms((p_teclado->tmr_duracion_columna), TIMEOUT_COLUMNA_TECLADO_MS);
}

//------------------------------------------------------
// FUNCIONES PROPIAS
//------------------------------------------------------
/* Getter y setters de variables globales */

TipoTecladoShared GetTecladoSharedVar(){
	piLock (KEYBOARD_KEY);  // fijarse bien en los mutex puede ser otro
	TipoTecladoShared copiaGetTecladoSharedVar = g_tecladoSharedVars;
	piUnlock (KEYBOARD_KEY);
	return copiaGetTecladoSharedVar;
}

void SetTecladoSharedVar(TipoTecladoShared value){
	piLock (KEYBOARD_KEY);
	g_tecladoSharedVars = value ;
	piUnlock (KEYBOARD_KEY);
}

void ActualizaExcitacionTecladoGPIO(int columna) {
	// ATENCION: Evitar que este mas de una columna activa a la vez.

	// A completar por el alumno
	// ...
	switch(columna){
	case COLUMNA_1:
			digitalWrite(GPIO_KEYBOARD_COL_1, HIGH);
			digitalWrite(GPIO_KEYBOARD_COL_2, LOW);
			digitalWrite(GPIO_KEYBOARD_COL_3, LOW);
			digitalWrite(GPIO_KEYBOARD_COL_4, LOW);
			g_tecladoSharedVars.columnaActual = COLUMNA_1;
			break;
	case COLUMNA_2:
			digitalWrite(GPIO_KEYBOARD_COL_1, LOW);
			digitalWrite(GPIO_KEYBOARD_COL_2, HIGH);
			digitalWrite(GPIO_KEYBOARD_COL_3, LOW);
			digitalWrite(GPIO_KEYBOARD_COL_4, LOW);
			g_tecladoSharedVars.columnaActual = COLUMNA_2;
			break;
	case COLUMNA_3:
			digitalWrite(GPIO_KEYBOARD_COL_1, LOW);
			digitalWrite(GPIO_KEYBOARD_COL_2, LOW);
			digitalWrite(GPIO_KEYBOARD_COL_3, HIGH);
			digitalWrite(GPIO_KEYBOARD_COL_4, LOW);
			g_tecladoSharedVars.columnaActual = COLUMNA_3;
			break;
	case COLUMNA_4:
			digitalWrite(GPIO_KEYBOARD_COL_1, LOW);
			digitalWrite(GPIO_KEYBOARD_COL_2, LOW);
			digitalWrite(GPIO_KEYBOARD_COL_3, LOW);
			digitalWrite(GPIO_KEYBOARD_COL_4, HIGH);
			g_tecladoSharedVars.columnaActual = COLUMNA_4;
			break;
	}
}

//------------------------------------------------------
// FUNCIONES DE ENTRADA O DE TRANSICION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------
int CompruebaTimeoutColumna(fsm_t* p_this) {


	int result = 0;
	piLock (KEYBOARD_KEY);
	result = (g_tecladoSharedVars.flags) & FLAG_TIMEOUT_COLUMNA_TECLADO;
	piUnlock (KEYBOARD_KEY);
	return result;
}


//------------------------------------------------------
// FUNCIONES DE SALIDA O DE ACCION DE LAS MAQUINAS DE ESTADOS
//------------------------------------------------------
void TecladoExcitaColumna(fsm_t* p_this) {

	TipoTeclado *p_teclado = (TipoTeclado*)(p_this->user_data);


	// 1. Actualizo que columna SE VA a excitar
	piLock(KEYBOARD_KEY);
	g_tecladoSharedVars.flags &= ~FLAG_TIMEOUT_COLUMNA_TECLADO;
	piUnlock(KEYBOARD_KEY);


	g_tecladoSharedVars.columnaActual = (g_tecladoSharedVars.columnaActual+1)%4;

	// 2. Ha pasado el timer y es hora de excitar la siguiente columna:
	//    (i) Llamada a ActualizaExcitacionTecladoGPIO con columna A ACTIVAR como argumento

	ActualizaExcitacionTecladoGPIO(g_tecladoSharedVars.columnaActual);

	// 3. Actualizar la variable flags

	piLock (KEYBOARD_KEY);
	g_tecladoSharedVars.flags |= FLAG_TIMEOUT_COLUMNA_TECLADO; /////////////////////
	piUnlock (KEYBOARD_KEY);

	// 4. Manejar el temporizador para que vuelva a avisarnos

	tmr_startms((p_teclado->tmr_duracion_columna), TIMEOUT_COLUMNA_TECLADO_MS);
	// A completar por el alumno
	// ...


}

//------------------------------------------------------
// SUBRUTINAS DE ATENCION A LAS INTERRUPCIONES
//------------------------------------------------------

static int row[4] = {0,0,0,0};
//static int col[4] = {0,0,0,0};

static void boton_isr (int b) {
	static unsigned antirrebotes[4]={0,0,0,0};
	int now = millis();

	if (now < antirrebotes[b]) return; // si se detecta una repeticion pulsada antes de que pase un tiempo, se la ignora

	char teclaRecibida = tecladoTL04[b][g_tecladoSharedVars.columnaActual];
	g_tecladoSharedVars.teclaDetectada = teclaRecibida;

	piLock (KEYBOARD_KEY);
	g_tecladoSharedVars.flags |= FLAG_TECLA_PULSADA;
	piUnlock (KEYBOARD_KEY);

	row[b]=1;
	antirrebotes[b] = now + DEBOUNCE_TIME_MS;
}

void teclado_fila_1_isr(void) {

	boton_isr (0);
}

void teclado_fila_2_isr(void) {

	boton_isr (1);
}

void teclado_fila_3_isr(void) {

	boton_isr (2);
}

void teclado_fila_4_isr (void) {


	boton_isr (3);
}




void timer_duracion_columna_isr(union sigval value) {
	// Simplemente avisa que ha pasado el tiempo para excitar la siguiente columna
		piLock (KEYBOARD_KEY);
		g_tecladoSharedVars.flags |= FLAG_TIMEOUT_COLUMNA_TECLADO;
		piUnlock (KEYBOARD_KEY);

}
