
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

const int SYSTEMS_COUNT = 3;
// Seleccione el indice del sistema. En este caso se toma el primer sistema
const int SYSTEM_SELETED = 0;
// Tamaño de cada uno de los sistemas. @TODO Se puede mejorar armando una struct
int systems_size[SYSTEMS_COUNT] = {3,3,5};
// Sistemas
sTaskParams taskParams[SYSTEMS_COUNT][10] = {
								 {{1,2,4,4}, {2,1,5,5},{3,1,6,6}}
								,{{1,3,5,5}, {2,1,7,7},{3,2,10,10}}
								,{{1,2,5,5}, {2,1,6,6},{3,1,7,7},{4,2,10,10},{5,1,15,15}}
								} ;



void thread1 (void*);
/* Implementar los siguientes conjuntos de tareas con FreeRTOS, bajo la polıtica
 * de planificacion (RM), y generar trazas de su ejecucion mediante Tracealyzer:
 * ejercicio 1. S(3) = {(2, 4, 4),(1, 5, 5),(1, 6, 6)}
 */
int main(){

	// Initializes the trace recorder, but does not start the tracing.	vTraceEnable( TRC_INIT );
	vTraceEnable( TRC_INIT );

	for (int i = 0; i <= systems_size[SYSTEM_SELETED] -1; i++){
		char name[10] = "";
		sprintf(name,"T%d",taskParams[SYSTEM_SELETED][i].id);
		xTaskCreate(thread1 					// Function that implements the task.
					,name						// Text name for the task.
					,256						// Stack size in words, not bytes.
					,(void*) &taskParams[SYSTEM_SELETED][i]		// Parameter passed into the task.
					,configMAX_PRIORITIES - (i+1)	// Priority at which the task is created.
					,NULL);						// Used to pass out the created task's handle.
	}

	// Starts the tracing.
	vTraceEnable( TRC_START );
	pc.printf("Iniciando simulación\n\r");

    vTaskStartScheduler();

    for( ;; );
}


void eat_cpu(int v,TickType_t ticks){
  TickType_t cnt = 0;
  TickType_t prev = xTaskGetTickCount();
  while (true) {
    if (prev < xTaskGetTickCount()) {
    	// se intenta prevenir los errores de salto.
		if (xTaskGetTickCount() - prev < 100) {
			cnt++;
		}
     	prev = xTaskGetTickCount();
    }
    if (cnt > ticks) {
      break;
    }
  }
}

char* get_state(int before, int after, int deadTime){
	if(after - before > deadTime){
		return "VENCIDA";
	}
	return "OK";
}
void thread1 (void* params){
    // Casteo de la estructura que contiene los parametros
    struct sTaskParams* taskParams;
    taskParams = (struct sTaskParams *) params;

    // Definicion de variables locales
    const portTickType xExecTime  =  taskParams->c * 1000;
    const portTickType xFrequency =  taskParams->t * 1000;
    const portTickType xDeadTime  =  taskParams->d * 1000;
    portTickType xLastWakeTime = 0;
    int32_t instance = 1;
    while(1){
    	int before = xTaskGetTickCount();
    	eat_cpu(before,xExecTime);
    	int after = xTaskGetTickCount();
    	pc.printf("Tarea %d [%d , %d, %d]  => %s\n\r"
    			,taskParams->id
    			,before
				,after
				,instance
				,get_state(before,after,(taskParams->d * 1000))
				);
    	instance = instance + 1;
    	if(instance == 5){
    		// Se agrega un breackpoint para que el sistema para en la instancia
    		// 5 de la primer tarea
    	}
	    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}

