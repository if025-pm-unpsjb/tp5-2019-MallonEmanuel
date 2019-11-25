
#include "mbed.h"       // Librería mbed
#include "FreeRTOS.h"   // Definiciones principales de FreeRTOS
#include "task.h"       // Funciones para el control de las tareas

struct sTaskParams{
	uint32_t id;
	uint32_t c;
	uint32_t d;
	uint32_t t;
};

void thread1 (void*);
/* Implementar los siguientes conjuntos de tareas con FreeRTOS, bajo la polıtica
 * de planificacion (RM), y generar trazas de su ejecucion mediante Tracealyzer:
 * ejercicio 1. S(3) = {(2, 4, 4),(1, 5, 5),(1, 6, 6)}
 */
int main(){

	// Initializes the trace recorder, but does not start the tracing.
	vTraceEnable( TRC_INIT );


	struct sTaskParams taskParams[4];
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

	int limit = 2;

/*	for (int i = 0; i <= limit ; i++){
		char name[10] = "";
		sprintf(name,"t_%d",i);
		xTaskCreate(thread1 					// Function that implements the task.
					,name						// Text name for the task.
					,128						// Stack size in words, not bytes.
					,(void*) &taskParams[i]		// Parameter passed into the task.
					,configMAX_PRIORITIES - (i+1)	// Priority at which the task is created.
					,NULL);						// Used to pass out the created task's handle.
	}
*/

	xTaskCreate(thread1 					// Function that implements the task.
					,"Tarea_A"						// Text name for the task.
					,256						// Stack size in words, not bytes.
					,(void*) &taskParams[0]		// Parameter passed into the task.
					,configMAX_PRIORITIES -1	// Priority at which the task is created.
					,NULL);						// Used to pass out the created task's handle.

	xTaskCreate(thread1 					// Function that implements the task.
						,"Tarea_I"						// Text name for the task.
						,128						// Stack size in words, not bytes.
						,(void*) &taskParams[1]		// Parameter passed into the task.
						,configMAX_PRIORITIES -2	// Priority at which the task is created.
						,NULL);						// Used to pass out the created task's handle.

	xTaskCreate(thread1 					// Function that implements the task.
							,"TAREA_Z"						// Text name for the task.
							,128						// Stack size in words, not bytes.
							,(void*) &taskParams[2]		// Parameter passed into the task.
							,configMAX_PRIORITIES -3 	// Priority at which the task is created.
							,NULL);						// Used to pass out the created task's handle.
	// Starts the tracing.
	vTraceEnable( TRC_START );

    vTaskStartScheduler();

    for( ;; );
}

void eat_cpu(int time){
	float res = 0;
	for(uint32_t i = 0;i <= time * 100; i++){
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
    const uint32_t time   		  = (uint32_t) taskParams->c;
    const portTickType xFrequency = (uint32_t) taskParams->t;
    portTickType xLastWakeTime = 0;

    while(1){
	    eat_cpu(time);
	    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}
