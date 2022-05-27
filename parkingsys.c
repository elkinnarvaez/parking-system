#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include "phtrdsMsgLyr.h"

/***( Function prototypes )***********************************************/

// static void *pCustomer ( void *arg );                   /* Customer code */
static void *pController ( void *arg );                 /* Controller code */
static void *pRequestParkingLogic ( void *arg );        /* Request Parking logic code */
static void *pAreaSensor ( void *arg );                 /* Area Sensor code */
static void *pDevice ( void* arg );                     /* Device code */

/***( SDL system creation )***********************************************/
int main (void) 
{
    srand(time(NULL));

    pthread_t  area_sensors[AREA_LOCS];
    pthread_t  cntrllr_tid;
    pthread_t  rprklog_tid;
    pthread_t  devices_tid[NUM_DEVICES];

    /* Create queues */
    initialiseQueues();

    /* Create threads */
    
    for ( int i = 0; i < AREA_LOCS; i++ ){ 
        pthread_create ( &area_sensors[i], NULL, pAreaSensor, (void *) &i );
        sleep(1);
    }

    pthread_create ( &cntrllr_tid, NULL, pController, NULL );
    pthread_create ( &rprklog_tid, NULL, pRequestParkingLogic, NULL );

    for ( int j = 2; j - 2 < NUM_DEVICES; j++ ) {
        pthread_create( &devices_tid[j], NULL, pDevice, (void *) &j );
        sleep(1);
    }

    /* Wait for threads to finish */
    for ( int i = 0; i < AREA_LOCS; i++ ) {
        pthread_join ( area_sensors[i], NULL );
    }

    for ( int j = 0; j < NUM_DEVICES; j++ ) {
        pthread_join ( devices_tid[j], NULL );
    }

    pthread_join ( cntrllr_tid, NULL );
    pthread_join ( rprklog_tid, NULL );

  /* Destroy queues */
    destroyQueues ();

    return ( 0 );
}

/***( Helper functions )**************************************************/

int randomValue ( int n, int m ) { 

    int ans = 0;
    for ( int i = 0; i < n; i++) {
        ans = rand() % m;
    }
    return ans;
}

/***( SDL system processes )**********************************************/

/* Device thread */
static void *pDevice ( void *arg ) {

    int randomValue ( int n, int m );

    int idUser;

    idUser = *((int *) arg);

    DEVICE_STATES options[2] = { Request,  WaitD },
                  option;
    
    msg_t InMsg,
          OutMsg;

    printf("[+] ID user: %d connected\n", idUser);
    fflush ( stdout );

    for ( ; ; ) {

        option = options[randomValue( 3, 2 )];

        switch (option)
        {
            case Request:

                OutMsg.signal = (int) sParkingRequest;
                OutMsg.value = idUser;
                sendMessage( &( queue[CONTROLLER_Q]), OutMsg );   

                InMsg = receiveMessage( &(queue [idUser]) );
                printf( "Information by User %d: ", InMsg.value);
                printf( "\nId Area\tCapacity\tSlotsAvaiable\tZone\n");
                for ( int i = 0; i < AREA_LOCS ; i++) {
                    printf("%d\t%d\t%d\t%d\n", i, InMsg.lots[i].capacity, InMsg.lots[i].slotsAvailable, InMsg.lots[i].zone);
                }
                fflush ( stdout );

                break;
            
            case WaitD:
                break;

            default:
                break;
        }
        
        sleep( (rand() % 5) + 1 );
        
    }


    return ( NULL );

}

/* Area Sensor thread */
static void *pAreaSensor ( void *arg ) {

    int randomValue ( int n, int m );

    int carsQuantity,
        *arg_ta,
        idArea;

    carsQuantity = 0;
    arg_ta = (int *) arg;
    idArea = *arg_ta;

    SENSORS_STATES initialOptions[2] = { Entry, Wait },
                   normalStates[4] = { Entry, Exit, Entry_Exit, Wait },
                   option;

    msg_t OutMsg,
          OutMsg2;

    printf("ID Area: %d Created\n", idArea);
    fflush ( stdout );

    for ( ; ; ) {

        if ( carsQuantity == 0 ) option = initialOptions[randomValue(3, 2)];
        else option = normalStates[randomValue(3, 4)];

        switch ( option )
        {
            case Entry:
                
                printf("Area %d: Entry car\n", idArea);
                fflush ( stdout );

                OutMsg.signal = (int) sEntryCar;
                OutMsg.value = idArea;
                sendMessage( &( queue[CONTROLLER_Q]), OutMsg );
                
                carsQuantity++;
                break;

            case Wait:
                break;
            
            case Exit:

                printf("Area %d: Exit car\n", idArea);
                fflush ( stdout );                

                OutMsg.signal = (int) sExitCar;
                OutMsg.value = idArea;
                sendMessage( &( queue[CONTROLLER_Q]), OutMsg );

                carsQuantity--;
                break;

            case Entry_Exit:

                printf("Area %d: Entry and exit car\n", idArea);
                fflush ( stdout );

                OutMsg.signal = (int) sEntryCar;
                OutMsg.value = idArea;
                sendMessage( &( queue[CONTROLLER_Q]), OutMsg );

                OutMsg2.signal = (int) sExitCar;
                OutMsg2.value = idArea;
                sendMessage( &( queue[CONTROLLER_Q]), OutMsg2 );

                break;

            default:
                break;
        }

        sleep((rand() % 4) + 1);

    }

    return ( NULL );
}


/* Controller thread */
static void *pController ( void *arg )
{
    CONTROLLER_STATES state,
                      state_next;
    msg_t             InMsg,
                      OutMsg;

    state_next = IdleC;
    for ( ; ; )
    {
        state = state_next;
        InMsg = receiveMessage( &(queue [CONTROLLER_Q]) );
        switch ( state )
        {
            case IdleC:
                
                switch ( InMsg.signal )
                {
                    case sEntryCar:

                        OutMsg.signal = (int) sAddNewCar;
                        OutMsg.value = InMsg.value;
                        sendMessage( &(queue [REQUEST_PARKING_LOGIC_Q]),OutMsg );
                        state_next = IdleC;
                        break;

                    case sExitCar:

                        OutMsg.signal = (int) sCarLeftParkingArea;
                        OutMsg.value = InMsg.value;
                        sendMessage( &(queue [REQUEST_PARKING_LOGIC_Q]), OutMsg );
                        state_next = IdleC;
                        break;
                        
                    case sParkingRequest:

                        OutMsg.signal = (int) sRetrieveAvailableParkingAreaLocs;
                        OutMsg.value = InMsg.value;
                        sendMessage( &(queue [REQUEST_PARKING_LOGIC_Q]), OutMsg );
                        state_next = IdleC;
                        break;

                    case sAvailableParkingAreaLocs:

                        OutMsg.signal = (int) sParkingAvailability;
                        OutMsg.value = InMsg.value;
                        OutMsg.lots = InMsg.lots;
                        sendMessage( &(queue [InMsg.value]), OutMsg );
                        state_next = IdleC;
                        break;

                    default:
                        break;
                }

                break;

            default:
                break;
        }
    }
    
    return ( NULL );
}

static void *pRequestParkingLogic( void *arg ) 
{
    PARKING_REQUEST_LOGIC_STATES state,
                                 state_next;
    msg_t                        InMsg,
                                 OutMsg;
    Area                         parking[AREA_LOCS];

    updateArea(0, 4, 4, 1, parking );
    updateArea(1, 3, 3, 2, parking );
    updateArea(2, 5, 5, 1, parking );
    updateArea(3, 2, 2, 2, parking );

    state_next = IdleP;

    for ( ; ; ) 
    {
        state = state_next;
        InMsg = receiveMessage( &(queue [REQUEST_PARKING_LOGIC_Q]) );
        switch (state)
        {
            case IdleP:
                
                switch ( InMsg.signal )
                {
                    case sAddNewCar:

                        parking[InMsg.value].slotsAvailable--;
                        state_next = IdleP;
                        break;

                    case sCarLeftParkingArea:
                        
                        if (parking[InMsg.value].slotsAvailable < parking[InMsg.value].capacity ) {
                            parking[InMsg.value].slotsAvailable++;
                        }
                        state_next = IdleP;
                        break;
                    
                    case sRetrieveAvailableParkingAreaLocs:

                        OutMsg.signal = (int) sAvailableParkingAreaLocs;
                        OutMsg.value = InMsg.value;
                        OutMsg.lots = parking;
                        sendMessage( &(queue[CONTROLLER_Q]), OutMsg );
                        state_next = IdleP;
                        break;

                    default:
                        break;
                }

                break;
            
            default:
                break;
        }

    }

    return ( NULL );
}