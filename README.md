# Parking System Project

## Commands

This project have some commands, these are:

### Create project

This command to allow you create all .o and execute files for you can deploy the project:

~~~
make
~~~

### Clean the project

Remove all .o and execute files of your folder created to this project

~~~
make clean
~~~

### Deploy the project

After create the project, run this command for the project start to deploy:

~~~
./ParkingSys
~~~

### Create a trace

When you want store a execution trace, just run this command and change the <name-file> tag by the file name trace that you want assign.

~~~
./ParkingSys | tee <name-file>
~~~

## Code explanation (Español)

Dentro de este projecto podrás encontrar los siguientes archivos:

~~~
Project_scaled_out
|-- parkingsys.c
|-- parkingsysTrc.c
|-- phtrdsMsgLyr.c
|-- phtrdsMsgLyr.h
|-- pMLusrConf.h
|-- README.md
~~~

De los cuales nos interesa ver los siguientes:

### Archivo: pMLusrConf.h

Este archivo contiene las constantes, tipos, estados, estructura del mensaje y señales de nuestro proyecto. De los cuales solamente se realizando los siguientes cambios:

~~~
13  #define     NUM_QUEUES                  4     /* number of queues */

...

16  #define     DEVICE_Q1                   2     /* queue 2: device 1 */
17  #define     DEVICE_Q2                   3     /* queue 3: device 2 */

...

22  #define NUM_DEVICES 2

...

79  /***( Sensors states )**********************************************************/
80  
81  typedef enum {
82    Exit,
83    Entry,
84    Entry_Exit,
85    Wait
86  } SENSORS_STATES;
87  
88  typedef enum {
89    Request,
90    WaitD
91  } DEVICE_STATES;
~~~

1. Se aumento el número de Colas para poder adicionar las 2 que estarán asociadas a los dispositivos.
2. Se crearon las colas asociadas a cada dispositivo.
3. Se definió el número de dispositivos que existirán dentro del sistema
4. Se definieron los estados para los sensores y dispositivos.

### Archivo: parkingsys.c

1. Se instanciaron las funciones que le serán asignadas a los hilos que representarán a los sensores y dispositivos.

    ~~~
    static void *pAreaSensor ( void *arg );
    static void *pDevice ( void* arg );
    ~~~
2. Se definieron las variables que almacenarán los hilos asociados a estos 2 agentes.

    ~~~
    pthread_t  area_sensors[AREA_LOCS];
    pthread_t  devices_tid[NUM_DEVICES];
    ~~~
3. Se construyeron los hilos.

    ~~~
    for ( int i = 0; i < AREA_LOCS; i++ ){ 
        pthread_create ( &area_sensors[i], NULL, pAreaSensor, (void *) &i );
        sleep(1);
    }
    ...
    for ( int j = 2; j - 2 < NUM_DEVICES; j++ ) {
        pthread_create( &devices_tid[j], NULL, pDevice, (void *) &j );
        sleep(1);
    }
    ~~~
4. Se define que debe esperar a que terminen los nuevos hilos-

    ~~~
    for ( int i = 0; i < AREA_LOCS; i++ ) {
        pthread_join ( area_sensors[i], NULL );
    }

    for ( int j = 0; j < NUM_DEVICES; j++ ) {
        pthread_join ( devices_tid[j], NULL );
    }
    ~~~
5. Se define una función que permite obtener un valor aleatorio dentro del rango de 0 a m, después de n iteraciones.

    ~~~
    int randomValue ( int n, int m ) { 

        int ans = 0;
        for ( int i = 0; i < n; i++) {
            ans = rand() % m;
        }
        return ans;
    }
    ~~~
6. Se define la función para los dispositivos:

    ~~~
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
    ~~~
7. Se define la función para los sensores.

    ~~~
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
    ~~~

8. Se define que cuando el `pController` reciba los datos de la request realizada por el dispositivo, este se los envie a la cola correspondiente.

    ~~~
    sendMessage( &(queue [InMsg.value]), OutMsg );
    ~~~

    Esto se logra, gracias a que el `InMsg.value` almacena el id del usuario y por el diseño de este sistema, el id de la cola es igual al id del usuario.