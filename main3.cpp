
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
	int from;
	int to;
};

const int SYSTEMS_COUNT = 3;
// Seleccione el indice del sistema. En este caso se toma el primer sistema
const int SYSTEM_SELETED = 0;
// Tamaño de cada uno de los sistemas. @TODO Se puede mejorar armando una struct
int systems_size[SYSTEMS_COUNT] = {3,3,5};
// Sistemas
sTaskParams taskParams[SYSTEMS_COUNT][10] = {
								 {{1,1,5,5,-1,2}, {2,1,5,5,1,3},{3,1,5,5,2,-1}}
								,{{1,1,5,5,-1,3}, {2,1,5,5,-1,3},{3,1,5,5,-1,-1}}
								,{{1,2,5,5,-1}, {2,1,6,6,-1},{3,1,7,7,-1},{4,2,10,10,-1},{5,1,15,15,-1}}
								} ;

int from[SYSTEMS_COUNT][5] = {};
//QueueHandle_t xQueue1to2,xQueue2to3;

QueueHandle_t xQueue[5][5];

void thread1 (void*);
/*
void thread2 (void*);
void thread3 (void*);
*/
int main(){

	// Initializes the trace recorder, but does not start the tracing.	vTraceEnable( TRC_INIT );	vTraceEnable( TRC_INIT );
	vTraceEnable( TRC_INIT );

	xTaskCreate(thread1 					// Function that implements the task.
				,"T1"						// Text name for the task.
				,512						// Stack size in words, not bytes.
				,(void*) &taskParams[SYSTEM_SELETED][0]		// Parameter passed into the task.
				,configMAX_PRIORITIES - 1	// Priority at which the task is created.
				,NULL);						// Used to pass out the created task's handle.

	xTaskCreate(thread1 					// Function that implements the task.
				,"T2"						// Text name for the task.
				,512						// Stack size in words, not bytes.
				,(void*) &taskParams[SYSTEM_SELETED][1]		// Parameter passed into the task.
				,configMAX_PRIORITIES - 2	// Priority at which the task is created.
				,NULL);						// Used to pass out the created task's handle.

	xTaskCreate(thread1 					// Function that implements the task.
				,"T3"						// Text name for the task.
				,512						// Stack size in words, not bytes.
				,(void*) &taskParams[SYSTEM_SELETED][2]		// Parameter passed into the task.
				,configMAX_PRIORITIES - 3	// Priority at which the task is created.
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
    int32_t instance = 0;
    //xQueue1to2 = xQueueCreate( 10, sizeof(int));
    if(taskParams->to > 0){
    	xQueue[taskParams->id][taskParams->to] = xQueueCreate( 10, sizeof(int));
    	if( xQueue[taskParams->id][taskParams->to] == 0 ){
			pc.printf("Error al crear la cola de la tarea %d\n\r",taskParams->id);
		}else{
			pc.printf("Cola de la tarea %d creada correctamente\n\r",taskParams->id);
		//	xQueueSend( xQueue1to2, &taskParams->id, ( TickType_t ) 0 );
		}
    }
    int msg_rec;
    int msg_send;
    int id = taskParams->id;
    while(1){
    	msg_rec = 0;
    	msg_send = 0;
    	int before = xTaskGetTickCount();
    	// si debe recibir mensajes los lee
		if( taskParams->from >0 && xQueue[taskParams->from][taskParams->id] != 0 ){
			if(xQueuePeek( xQueue[taskParams->from][taskParams->id], &msg_rec, ( TickType_t ) 10 ) ){
			//	pc.printf("[%d] recibio mensaje de %d\n\r",taskParams->id,msg_rec);
			}
		}
		eat_cpu(xExecTime);
		// si tiene que enviar ,mensajes los envia
		if( xQueue[taskParams->id][taskParams->to] != 0){
			xQueueSend( xQueue[taskParams->id][taskParams->to], &id, ( TickType_t ) 0 );
			msg_send = taskParams->id;
			//pc.printf("[%d] enviando mensaje a %d\n\r",taskParams->id,taskParams->to);
		}

    	int after = xTaskGetTickCount();
    	pc.printf("Tarea %d [%d , %d, %d] Recibido: %d  Enviado:%d \n\r"
    			,taskParams->id
    			,before
				,after
				,instance
				,msg_rec
				,msg_send
				);
    	instance = instance + 1;
 	    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}

/*

void thread2 (void* params){
    // Casteo de la estructura que contiene los parametros
    struct sTaskParams* taskParams;
    taskParams = (struct sTaskParams *) params;

    // Definicion de variables locales
    const portTickType xExecTime  =  taskParams->c * 1000;
    const portTickType xFrequency =  taskParams->t * 1000;
    //const portTickType xDeadTime  =  taskParams->d * 1000;
    portTickType xLastWakeTime = 0;
    int32_t instance = 0;
    xQueue2to3 = xQueueCreate( 10, sizeof(int));
    if( xQueue2to3 == 0 ){
		pc.printf("Error al crear la cola de la tarea %d\n\r",taskParams->id);
	}else{
		pc.printf("Cola de la tarea %d creada correctamente\n\r",taskParams->id);
	//	xQueueSend( xQueue1to2, &taskParams->id, ( TickType_t ) 0 );
	}
    int msg;
    while(1){
    	msg = 0;
    	int before = xTaskGetTickCount();
    	eat_cpu(xExecTime);
		if( xQueue[1][taskParams->id] != 0 ){
			if(xQueuePeek( xQueue[1][taskParams->id], &msg, ( TickType_t ) 10 ) ){
				pc.printf("[%d] recibio mje de %d\n\r",taskParams->id,msg);
				xQueueSend( xQueue2to3, &taskParams->id, ( TickType_t ) 0 );
			}
		}
        int after = xTaskGetTickCount();
    	pc.printf("Tarea %d [%d , %d, %d] %d => %s\n\r"
    			,taskParams->id
    			,before
				,after
				,instance
				,(after - (instance * xFrequency))
				,get_state((instance * xFrequency),after,(taskParams->d * 1000))
				);
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
    //const portTickType xDeadTime  =  taskParams->d * 1000;
    portTickType xLastWakeTime = 0;
    int32_t instance = 0;
    int msg;
    while(1){
    	int before = xTaskGetTickCount();
    	eat_cpu(xExecTime);
    	if( xQueue2to3 != 0 ){
			if(xQueuePeek( xQueue2to3, &msg, ( TickType_t ) 10 ) ){
				pc.printf("[%d] recibio mje de %d\n\r",taskParams->id,msg);
			}
		}
    	int after = xTaskGetTickCount();
    	pc.printf("Tarea %d [%d , %d, %d] %d => %s\n\r"
    			,taskParams->id
    			,before
				,after
				,instance
				,(after - (instance * xFrequency))
				,get_state((instance * xFrequency),after,(taskParams->d * 1000))
				);
    	instance = instance + 1;
 	    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}
*/
