
#include "mbed.h"       // Librería mbed
#include "FreeRTOS.h"   // Definiciones principales de FreeRTOS
#include "task.h"       // Funciones para el control de las tareas

#include "queue.h"
#include "semphr.h"

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
								 {{1,1,5,5}, {2,1,5,5},{3,1,5,5}}
								,{{1,1,5,5}, {2,1,5,5},{3,1,5,5}}
								,{{1,2,10,10}, {2,1,10,10},{3,1,10,10},{4,2,10,10},{5,1,10,10}}
								} ;

QueueHandle_t xQueue1to2,xQueue2to3;
SemaphoreHandle_t xSemaphore = 0;

void thread1 (void*);

void thread2 (void*);
void thread3 (void*);

int main(){

	// Initializes the trace recorder, but does not start the tracing.	vTraceEnable( TRC_INIT );	vTraceEnable( TRC_INIT );
	vTraceEnable( TRC_INIT );
	xSemaphore = xSemaphoreCreateMutex(); // @suppress("Function cannot be resolved")
	xTaskCreate(thread1 					// Function that implements the task.
				,"T1"						// Text name for the task.
				,512						// Stack size in words, not bytes.
				,(void*) &taskParams[SYSTEM_SELETED][0]		// Parameter passed into the task.
				,configMAX_PRIORITIES - 1	// Priority at which the task is created.
				,NULL);						// Used to pass out the created task's handle.

	xTaskCreate(thread2 					// Function that implements the task.
				,"T2"						// Text name for the task.
				,512						// Stack size in words, not bytes.
				,(void*) &taskParams[SYSTEM_SELETED][1]		// Parameter passed into the task.
				,configMAX_PRIORITIES - 1	// Priority at which the task is created.
				,NULL);						// Used to pass out the created task's handle.

	xTaskCreate(thread3 					// Function that implements the task.
				,"T3"						// Text name for the task.
				,512						// Stack size in words, not bytes.
				,(void*) &taskParams[SYSTEM_SELETED][2]		// Parameter passed into the task.
				,configMAX_PRIORITIES - 1	// Priority at which the task is created.
				,NULL);						// Used to pass out the created task's handle.

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
    //const portTickType xDeadTime  =  taskParams->d * 1000;
    portTickType xLastWakeTime = 0;
    int32_t instance = 1;
    xQueue1to2 = xQueueCreate( 10, sizeof(int)); // @suppress("Function cannot be resolved")
    int msg_rec;
    int id = taskParams->id;
    while(1){
    	msg_rec = 0;
    	int before = xTaskGetTickCount();
    	// si tiene que enviar ,mensajes los envia
		if( xQueue1to2 != 0){
				int after = xTaskGetTickCount();
				xQueueSend( xQueue1to2, &id, ( TickType_t ) 0 ); // @suppress("Invalid arguments")
				eat_cpu(xExecTime);
				if(xSemaphoreTake(xSemaphore, 5000) == pdTRUE){ // @suppress("Invalid arguments")
					pc.printf("Tarea %d [%d , %d, %d] Enviando= %d\n\r"
							,taskParams->id
							,before
							,after
							,instance
							,taskParams->id
							);
					xSemaphoreGive(xSemaphore); // @suppress("Invalid arguments")
				}
		}
    	instance = instance + 1;
 	    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}



void thread2 (void* params){
    // Casteo de la estructura que contiene los parametros
    struct sTaskParams* taskParams;
    taskParams = (struct sTaskParams *) params;

    // Definicion de variables locales
    const portTickType xExecTime  =  taskParams->c * 1000;
    const portTickType xFrequency =  taskParams->t * 1000;
    const portTickType xDeadTime  =  taskParams->d * 1000;
    const int id = taskParams->id;
    portTickType xLastWakeTime = 0;
    int32_t instance = 1;
    xQueue2to3 = xQueueCreate( 10, sizeof(int)); // @suppress("Function cannot be resolved")

    int msg;
    while(1){
    	msg = 0;

    	//if(xQueuePeek( xQueue[1][taskParams->id], &msg, ( TickType_t ) 10 ) ){
    	if( xQueue2to3 != 0 ){
			if(xQueueReceive( xQueue1to2, &msg, ( TickType_t ) xDeadTime ) == pdTRUE){ // @suppress("Invalid arguments")
				int before = xTaskGetTickCount();
				eat_cpu(xExecTime);
				xQueueSend( xQueue2to3, &id, ( TickType_t ) 0 ); // @suppress("Invalid arguments")
				int after = xTaskGetTickCount();
				if(xSemaphoreTake(xSemaphore, 5000) == pdTRUE){ // @suppress("Invalid arguments")
					pc.printf("Tarea %d [%d , %d, %d] Recibido=%d Enviado a= %d\n\r"
							,taskParams->id
							,before
							,after
							,instance
							,msg
							,3
							);
					xSemaphoreGive(xSemaphore);
				}
			}
    	}
    	instance = instance + 1;
    	vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}

void thread3 (void* params){
    // Casteo de la estructura que contiene los parametros
    struct sTaskParams* taskParams;
    taskParams = (struct sTaskParams *) params;

    // Definicion de variables locales
    const portTickType xExecTime  =  taskParams->c * 1000;
    const portTickType xFrequency =  taskParams->t * 1000;
    const portTickType xDeadTime  =  taskParams->d * 1000;
    portTickType xLastWakeTime = 0;
    int32_t instance = 1;
    int msg;
    while(1){
    	int before = xTaskGetTickCount();
    	if( xQueue2to3 != 0 ){
			if(xQueueReceive( xQueue2to3, &msg, ( TickType_t ) xDeadTime )  == pdTRUE){ // @suppress("Invalid arguments")

				eat_cpu(xExecTime);
				int after = xTaskGetTickCount();
				if(xSemaphoreTake(xSemaphore, 5000) == pdTRUE){ // @suppress("Invalid arguments")
					pc.printf("Tarea %d [%d , %d, %d] Recibido=%d \n\r"
							,taskParams->id
							,before
							,after
							,instance
							,msg
							);
					xSemaphoreGive(xSemaphore);
				}
			}
		}
    	instance = instance + 1;
 	    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}
