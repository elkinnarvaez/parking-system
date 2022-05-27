/*******************************************************************************
*
*  pMLusrConf.h -   Manifest Constants and Types for concurrent access to a
*                   circular buffer modelling a message queue
*
*   Notes:          User defined according to application
*
*******************************************************************************/

/***( Manifest constants for fser-defined queuing system  )********************/

#define     BUFSIZE                     8     /* number of slots in queues */
#define     NUM_QUEUES                  4     /* number of queues */
#define     CONTROLLER_Q                0     /* queue 0: controller */
#define     REQUEST_PARKING_LOGIC_Q     1     /* queue 1: request parking logic */
#define     DEVICE_Q1                   2     /* queue 2: device 1 */
#define     DEVICE_Q2                   3     /* queue 3: device 2 */

/***( User-defined types )*****************************************************/

#define AREA_LOCS 4
#define NUM_DEVICES 2

typedef struct 
{
  int capacity;
  int slotsAvailable;
  int zone;
} Area;

void updateArea( int, int, int, int, Area* );

/***( User-defined message structure )*****************************************/

typedef struct
{
  int   signal;
  int   value;
  Area  *lots;
} msg_t;

/***( User-defined signals )****************************************************/

typedef enum
{
  sParkingAvailability
} TO_CUSTOMER ;                           /* Signals sent to customer
                                              (environment) */

typedef enum
{
  sParkingRequest,                                  /*  Signals sent    */
  sEntryCar,                                        /*    from customer */
  sExitCar,                                         /*    to controller */
  sAvailableParkingAreaLocs                         /*  Signal from parking request logic to controller */
} TO_CONTROLLER;


typedef enum
{
  sRetrieveAvailableParkingAreaLocs,                /*   Signals sent   */
  sAddNewCar,                                       /* from controller  */
  sCarLeftParkingArea                               /* to parking request logic */
} TO_PARKING_REQUEST_LOGIC;                            


/***( User-defined EFSM states )************************************************/

typedef enum
{
  IdleC
} CONTROLLER_STATES;                      /* EFSM states for controller */

typedef enum
{
  IdleP
} PARKING_REQUEST_LOGIC_STATES;                        /* EFSM states for parking request logic */

/***( Sensors states )**********************************************************/

typedef enum {
  Exit,
  Entry,
  Entry_Exit,
  Wait
} SENSORS_STATES;

typedef enum {
  Request,
  WaitD
} DEVICE_STATES;