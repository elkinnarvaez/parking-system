#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "phtrdsMsgLyr.h"

/***( Function prototypes )***********************************************/

static void *pCustomer(void *arg);            /* Customer code */
static void *pController(void *arg);          /* Controller code */
static void *pRequestParkingLogic(void *arg); /* Request Parking logic code */

/***( SDL system creation )***********************************************/
int main(void)
{

    pthread_t customr_tid;
    pthread_t cntrllr_tid;
    pthread_t rprklog_tid;

    /* Create queues */
    initialiseQueues();

    /* Create threads */
    pthread_create(&customr_tid, NULL, pCustomer, NULL);
    pthread_create(&cntrllr_tid, NULL, pController, NULL);
    pthread_create(&rprklog_tid, NULL, pRequestParkingLogic, NULL);

    /* Wait for threads to finish */
    pthread_join(customr_tid, NULL);
    pthread_join(cntrllr_tid, NULL);
    pthread_join(rprklog_tid, NULL);

    /* Destroy queues */
    destroyQueues();

    return (0);
}

/***( Helper functions )**************************************************/

void menu()
{
    printf("Choice an option:\n");
    printf("  1. Entry a car.\n");
    printf("  2. Exit a car.\n");
    printf("  3. Parking status.\n\n");
    printf("Option: ");
}

/***( SDL system processes )**********************************************/

/* Customer thread */
static void *pCustomer(void *arg)
{
    char line[100];
    int option,
        idArea,
        idUser;
    msg_t OutMsg;

    printf("[pCustomer]: Created \n");
    fflush(stdout);

    for (;;)
    {
        menu();
        fflush(stdout);
        fflush(stdin);
        fgets(line, sizeof(line), stdin);
        sscanf(line, "%d", &option);

        switch (option)
        {
        case 1:

            printf("\nID Area: ");
            fflush(stdout);
            fflush(stdin);
            fgets(line, sizeof(line), stdin);
            sscanf(line, "%d", &idArea);

            OutMsg.signal = (int)sEntryCar;
            OutMsg.value = idArea;
            sendMessage(&(queue[CONTROLLER_Q]), OutMsg);

            printf("[pCustomer] Emited: sEntryCar(idArea=%d) \n", idArea);
            fflush(stdout);

            break;

        case 2:

            printf("\nID Area: ");
            fflush(stdout);
            fflush(stdin);
            fgets(line, sizeof(line), stdin);
            sscanf(line, "%d", &idArea);

            OutMsg.signal = (int)sExitCar;
            OutMsg.value = idArea;
            sendMessage(&(queue[CONTROLLER_Q]), OutMsg);

            printf("[pCustomer] Emited: sExitCar(idArea=%d) \n", idArea);
            fflush(stdout);

            break;

        case 3:

            printf("\nID User: ");
            fflush(stdout);
            fflush(stdin);
            fgets(line, sizeof(line), stdin);
            sscanf(line, "%d", &idUser);

            OutMsg.signal = (int)sParkingRequest;
            OutMsg.value = idUser;
            sendMessage(&(queue[CONTROLLER_Q]), OutMsg);

            printf("[pCustomer] Emited: sParkingRequest(idUser=%d) \n", idUser);
            fflush(stdout);

            break;

        default:
            break;
        }
    }
    return (NULL);
}

/* Controller thread */
static void *pController(void *arg)
{
    CONTROLLER_STATES state,
        state_next;
    msg_t InMsg,
        OutMsg;

    state_next = IdleC;

    printf("[pController]: Created\n");
    fflush ( stdout );

    for ( ; ; )
    {
        state = state_next;
        InMsg = receiveMessage(&(queue[CONTROLLER_Q]));

        switch (state)
        {
        case IdleC:

            switch (InMsg.signal)
            {
            case sEntryCar:

                printf("[pController] Received: Signal sEntryCar \n");
                fflush(stdout);

                OutMsg.signal = (int)sAddNewCar;
                OutMsg.value = InMsg.value - 1;
                sendMessage(&(queue[REQUEST_PARKING_LOGIC_Q]), OutMsg);
                state_next = IdleC;

                printf("[pController] Emited: (IdleC) sAddNewCar(idArea=%d) \n", InMsg.value);
                fflush(stdout);

                break;

            case sExitCar:

                printf("[pController] Received: Signal sExitCar \n");
                fflush(stdout);

                OutMsg.signal = (int)sCarLeftParkingArea;
                OutMsg.value = InMsg.value - 1;
                sendMessage(&(queue[REQUEST_PARKING_LOGIC_Q]), OutMsg);
                state_next = IdleC;

                printf("[pController] Emited: (IdleC) sAddNewCar(idArea=%d) \n", InMsg.value);
                fflush(stdout);

                break;

            case sParkingRequest:

                printf("[pController] Received: Signal sParkingRequest \n");
                fflush(stdout);

                OutMsg.signal = (int)sRetrieveAvailableParkingAreaLocs;
                OutMsg.value = InMsg.value;
                sendMessage(&(queue[REQUEST_PARKING_LOGIC_Q]), OutMsg);
                state_next = IdleC;

                printf("[pController] Emited: (IdleC) sRetrieveAvailableParkingAreaLocs(idUser=%d) \n", InMsg.value);
                fflush(stdout);

                break;

            case sAvailableParkingAreaLocs:

                printf("[pController] Received: Signal sAvailableParkingAreaLocs \n");
                fflush(stdout);

                OutMsg.signal = (int)sParkingAvailability;
                OutMsg.value = InMsg.value;
                OutMsg.lots = InMsg.lots;

                printf("[pController] Emited: (IdleC) sParkingAvailability(parkingAreaLocation, idUser=%d) \n", InMsg.value);
                fflush(stdout);

                printf("Information by User %d: ", InMsg.value);
                printf("\nId Area\tCapacity\tSlotsAvaiable\tZone\n");
                for (int i = 0; i < AREA_LOCS; i++)
                {
                    printf("%d\t%d\t%d\t%d\n", i+1, OutMsg.lots[i].capacity, OutMsg.lots[i].slotsAvailable, OutMsg.lots[i].zone);
                }
                fflush(stdout);

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

static void *pRequestParkingLogic(void *arg)
{
    PARKING_REQUEST_LOGIC_STATES state,
        state_next;
    msg_t InMsg,
        OutMsg;
    Area parking[AREA_LOCS];

    updateArea(0, 4, 4, 1, parking);
    updateArea(1, 3, 3, 2, parking);
    updateArea(2, 5, 5, 1, parking);
    updateArea(3, 2, 2, 2, parking);

    state_next = IdleP;

    printf("[pRequestParkingLogic]: Created\n");
    fflush(stdout);

    for (;;)
    {
        state = state_next;
        InMsg = receiveMessage(&(queue[REQUEST_PARKING_LOGIC_Q]));
        switch (state)
        {
        case IdleP:

            switch (InMsg.signal)
            {
            case sAddNewCar:

                printf("[pRequestParkingLogic] Received: Signal sAddNewCar \n");
                fflush(stdout);

                parking[InMsg.value].slotsAvailable--;
                state_next = IdleP;
                break;

            case sCarLeftParkingArea:

                printf("[pRequestParkingLogic] Received: Signal sCarLeftParkingArea \n");
                fflush(stdout);

                if (parking[InMsg.value].slotsAvailable < parking[InMsg.value].capacity)
                {
                    parking[InMsg.value].slotsAvailable++;
                }
                state_next = IdleP;
                break;

            case sRetrieveAvailableParkingAreaLocs:

                printf("[pRequestParkingLogic] Received: Signal sRetrieveAvailableParkingAreaLocs\n");
                fflush(stdout);

                OutMsg.signal = (int)sAvailableParkingAreaLocs;
                OutMsg.value = InMsg.value;
                OutMsg.lots = parking;
                sendMessage(&(queue[CONTROLLER_Q]), OutMsg);
                state_next = IdleP;

                printf("[pRequestParkingLogic] Emited: (IdleP) sAvailableParkingAreaLocs(parkingAreaLocation, idUser=%d) \n", InMsg.value);
                fflush(stdout);

                break;

            default:
                break;
            }

            break;

        default:
            break;
        }
    }
}