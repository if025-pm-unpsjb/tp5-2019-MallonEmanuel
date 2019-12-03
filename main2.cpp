
#include "mbed.h"       // Librería mbed
#include "FreeRTOS.h"   // Definiciones principales de FreeRTOS
#include "task.h"       // Funciones para el control de las tareas
#include "semphr.h"

Serial pc(USBTX, USBRX); // tx, rx

struct sTaskParams{
	int id;
	int c;
	int d;
	int t;
	int wait;
};

// Sistema
sTaskParams taskParams[] = {{1,1,4,4,3000}, {2,1,5,5,3000},{3,3,10,10,5000}};
int system_size = 3;

SemaphoreHandle_t xSemaphore = 0;

void thread1 (void*);

int main(){

	// Initializes the trace recorder, but does not start the tracing.	vTraceEnable( TRC_INIT );
	vTraceEnable( TRC_INIT );
	xSemaphore = xSemaphoreCreateMutex();
	for (int i = 0; i < system_size; i++){
		char name[10] = "";
		sprintf(name,"T%d",taskParams[i].id);
		xTaskCreate(thread1 					// Function that implements the task.
					,name						// Text name for the task.
					,256						// Stack size in words, not bytes.
					,(void*) &taskParams[i]		// Parameter passed into the task.
					,configMAX_PRIORITIES - (i+1)	// Priority at which the task is created.
					,NULL);						// Used to pass out the created task's handle.
	}

	// Starts the tracing.
	vTraceEnable( TRC_START );
	pc.printf("Iniciando simulación\n\r");

    vTaskStartScheduler();

    for( ;; );
}


void eat_cpu(TickType_t ticks){
  TickType_t cnt = 0;
  TickType_t prev = xTaskGetTickCount();
  while (cnt < (ticks - 50)) {
    if (prev < xTaskGetTickCount()) {
    	cnt++;
	 	prev = xTaskGetTickCount();
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
        if(xSemaphoreTake(xSemaphore, taskParams->wait-50) == pdTRUE){
        	pc.printf("Tarea %d[%d] tomo el recurso. %d\n\r",taskParams->id,instance,xTaskGetTickCount());
        	eat_cpu(xExecTime);
        	pc.printf("Tarea %d[%d] solto el recurso. %d\n\r",taskParams->id,instance,xTaskGetTickCount());
        	xSemaphoreGive(xSemaphore);
        }else{
        	pc.printf("Tarea %d[%d] no puedo tomar el recurso. %d\n\r",taskParams->id,instance,xTaskGetTickCount());
        }
    	instance = instance + 1;
 	    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}

