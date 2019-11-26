
#include "mbed.h"       // Librería mbed
#include "FreeRTOS.h"   // Definiciones principales de FreeRTOS
#include "task.h"       // Funciones para el control de las tareas

Serial pc(USBTX, USBRX); // tx, rx

struct sTaskParams{
	int id;
	int c;
	int d;
	int t;
};

sTaskParams taskParams[3] =  { { 0, 2, 4, 4 }, { 1, 1, 5, 5 },
		{ 2, 1, 6, 6 } };


void thread1 (void*);
/* Implementar los siguientes conjuntos de tareas con FreeRTOS, bajo la polıtica
 * de planificacion (RM), y generar trazas de su ejecucion mediante Tracealyzer:
 * ejercicio 1. S(3) = {(2, 4, 4),(1, 5, 5),(1, 6, 6)}
 */
int main(){

	// Initializes the trace recorder, but does not start the tracing.
	vTraceEnable( TRC_INIT );
/*	struct sTaskParams taskParams[4];
	taskParams[0].id = 0;
	taskParams[0].c = 2;
	taskParams[0].d = 4;
	taskParams[0].t = 4;

	taskParams[1].id = 1;
	taskParams[1].c = 1;
	taskParams[1].d = 5;
	taskParams[1].t = 5;

	taskParams[2].id = 2;
	taskParams[2].c = 1;
	taskParams[2].d = 6;
	taskParams[2].t = 6;
*/
	int limit = 2;

	for (int i = 0; i <= limit ; i++){
		char name[10] = "";
		sprintf(name,"T%d",i);
		xTaskCreate(thread1 					// Function that implements the task.
					,name						// Text name for the task.
					,256						// Stack size in words, not bytes.
					,(void*) &taskParams[i]		// Parameter passed into the task.
					,configMAX_PRIORITIES - (i+1)	// Priority at which the task is created.
					,NULL);						// Used to pass out the created task's handle.
	}

/*
	xTaskCreate(thread1 					// Function that implements the task.
					,"Tarea_A"						// Text name for the task.
					,256						// Stack size in words, not bytes.
					,(void*) &taskParams[0]		// Parameter passed into the task.
					,configMAX_PRIORITIES -2	// Priority at which the task is created.
					,NULL);						// Used to pass out the created task's handle.

	xTaskCreate(thread1 					// Function that implements the task.
						,"Tarea_I"						// Text name for the task.
						,256						// Stack size in words, not bytes.
						,(void*) &taskParams[1]		// Parameter passed into the task.
						,configMAX_PRIORITIES -3	// Priority at which the task is created.
						,NULL);						// Used to pass out the created task's handle.

	xTaskCreate(thread1 					// Function that implements the task.
							,"TAREA_Z"						// Text name for the task.
							,256						// Stack size in words, not bytes.
							,(void*) &taskParams[2]		// Parameter passed into the task.
							,configMAX_PRIORITIES -4 	// Priority at which the task is created.
							,NULL);						// Used to pass out the created task's handle.
*/
	// Starts the tracing.
	vTraceEnable( TRC_START );
	pc.printf("Iniciando simulación\n\r");

    vTaskStartScheduler();

    for( ;; );
}

void eat_cpu(uint32_t time){
	float res = 0;
	for(uint32_t i = 0;i <= time ; i++){
		for(uint32_t j; j <= 500 ; j++){
			res = j * (j+1) * 100 + (100 *10 * 2);
			res = j * (j+1) * 100 + (100 *10 * 2);
		}
	}
}

void thread1 (void* params){
    // Casteo de la estructura que contiene los parametros
    struct sTaskParams* taskParams;
    taskParams = (struct sTaskParams *) params;

    // Definicion de variables locales
    const portTickType time   	  =  taskParams->c * 1000;
    const portTickType xFrequency =  taskParams->t * 1000;
    portTickType xLastWakeTime = 0;

    while(1){
    	pc.printf("Ejecutando tarea %d durante %d\n\r",taskParams->id,taskParams->c);
	    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}

