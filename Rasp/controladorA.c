/**********************************************************
 *  INCLUDES
 *********************************************************/
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include <rtems.h>
#include <bsp.h>

#ifdef RASPBERRYPI
#include <bsp/i2c.h>
#endif

#include "displayA.h"

/**********************************************************
 *  Constants
 **********************************************************/
#define MSG_LEN    8
#define SLAVE_ADDR 0x8
#define NS_PER_S  1000000000
/**********************************************************
 *  Global Variables
 *********************************************************/
float speed = 0.0;
struct timespec time_msg = {0,400000000};
struct timespec time_display ={0,500000000};
int fd_i2c = -1;
struct timespec start_timeMixer;
struct timespec end_timeMixer;
struct timespec diff_timeMixer;
short mixer_state=0; //0=off,1=on
struct timespec start_rest_time_secondary_cycle;
struct timespec end_rest_time_secondary_cycle;
struct timespec diff_rest_time_secondary_cycle;
struct timespec to_sleep_loop;


void diffTimeTest(struct timespec end,
                  struct timespec start,
                  struct timespec *diff)
{
    if (end.tv_nsec < start.tv_nsec) {
        diff->tv_nsec = NS_PER_S - start.tv_nsec + end.tv_nsec;
        diff->tv_sec = end.tv_sec - (start.tv_sec+1);
    } else {
        diff->tv_nsec = end.tv_nsec - start.tv_nsec;
        diff->tv_sec = end.tv_sec - start.tv_sec;
    }
}

/**********************************************************
 *  Function: task_speed
 *********************************************************/
int task_speed()
{
    char request[10];
    char answer[10];

    //--------------------------------
    //  request speed and display it
    //--------------------------------

    //clear request and answer
    memset(request, '\0', 10);
    memset(answer, '\0', 10);

    // request speed
    strcpy(request, "SPD: REQ\n");

#ifdef RASPBERRYPI
    // use Raspberry Pi I2C serial module
    write(fd_i2c, request, MSG_LEN);
    nanosleep(&time_msg, NULL);
    read(fd_i2c, answer, MSG_LEN);
#else
    //Use the simulator
    simulator(request, answer);
#endif

    // display speed
    if (1 == sscanf (answer, "SPD:%f\n", &speed)){
        displaySpeed(speed);
        nanosleep(&time_display, NULL);
    }
    return 0;
}

//-------------------------------------
//-  Function: task_slope
//-------------------------------------
int task_slope()
{
    char request[10];
    char answer[10];

    //--------------------------------
    //  request slope and display it
    //--------------------------------

    //clear request and answer
    memset(request,'\0',10);
    memset(answer,'\0',10);

    // request slope
    strcpy(request, "SLP: REQ\n");
    //TODO

#ifdef RASPBERRYPI
    // use Raspberry Pi I2C serial module
    write(fd_i2c, request, MSG_LEN);
    nanosleep(&time_msg, NULL);
    read(fd_i2c, answer, MSG_LEN);
#else
    //Use the simulator
    simulator(request, answer);
#endif

    // display slope
    if (0 == strcmp(answer, "SLP:DOWN\n")) displaySlope(-1);
    if (0 == strcmp(answer, "SLP:FLAT\n")) displaySlope(0);
    if (0 == strcmp(answer, "SLP:  UP\n")) displaySlope(1);

    return 0;
}

void task_gas(){

    char request[10];
    char answer[10];

    //clear request and answer
    memset(request,'\0',10);
    memset(answer,'\0',10);

    if (speed <55){
        strcpy(request, "GAS: SET\n");
        displayGas(1);
        nanosleep(&time_display, NULL);
    }
    else{
        strcpy(request, "GAS: CLR\n");
        displayGas(0);
        nanosleep(&time_display, NULL);
    }

#ifdef RASPBERRYPI
    // use Raspberry Pi I2C serial module
  write(fd_i2c, request, MSG_LEN);
  nanosleep(&time_msg, NULL);
  read(fd_i2c, answer, MSG_LEN);
#else
    //Use the simulator
    simulator(request, answer);
#endif

}

void task_brake(){

    char request[10];
    char answer[10];

    //clear request and answer
    memset(request,'\0',10);
    memset(answer,'\0',10);

    if (speed <55){
        strcpy(request, "BRK: CLR\n");
        displayBrake(0);
        nanosleep(&time_display, NULL);
    }
    else{
        strcpy(request, "BRK: SET\n");
        displayBrake(1);
        nanosleep(&time_display, NULL);
    }

#ifdef RASPBERRYPI
    // use Raspberry Pi I2C serial module
  write(fd_i2c, request, MSG_LEN);
  nanosleep(&time_msg, NULL);
  read(fd_i2c, answer, MSG_LEN);
#else
    //Use the simulator
    simulator(request, answer);
#endif

}

void task_mixer(){
    clock_gettime(CLOCK_MONOTONIC,&end_timeMixer);
    int diff = end_timeMixer.tv_sec - start_timeMixer.tv_sec;
    if (diff>30){

        char request[10];
        char answer[10];

        //clear request and answer
        memset(request,'\0',10);
        memset(answer,'\0',10);

        if (mixer_state==0){
            mixer_state=1;
            strcpy(request, "MIX: SET\n");
            displayMix(1);
            nanosleep(&time_display, NULL);
        }
        else{
            mixer_state=0;
            strcpy(request, "MIX: CLR\n");
            displayMix(0);
            nanosleep(&time_display, NULL);
        }

#ifdef RASPBERRYPI
        // use Raspberry Pi I2C serial module
    write(fd_i2c, request, MSG_LEN);
    nanosleep(&time_msg, NULL);
    read(fd_i2c, answer, MSG_LEN);
#else
        //Use the simulator
        simulator(request, answer);
#endif

        clock_gettime(CLOCK_MONOTONIC,&start_timeMixer);
    }


}

//-------------------------------------
//-  Function: controller
//-------------------------------------
void *controller(void *arg)
{
    clock_gettime(CLOCK_MONOTONIC,&start_timeMixer);
    // Endless loop
    while(1) {
        clock_gettime(CLOCK_MONOTONIC,&start_rest_time_secondary_cycle);
        //1st secondary cycle
        task_speed();
        task_slope();
        task_gas();
        task_brake();
        task_mixer();
        sleep(5);//wait rest of secondary cycle

        //2nd secondary cycle
        task_speed();
        task_slope();
        task_gas();
        task_brake();
        sleep(5);//wait rest of secondary cycle
        task_mixer();

        //3rd secondary cycle
        task_speed();
        task_slope();
        task_gas();
        task_brake();


        clock_gettime(CLOCK_MONOTONIC,&end_rest_time_secondary_cycle);
        diffTimeTest(end_rest_time_secondary_cycle,start_rest_time_secondary_cycle,&diff_rest_time_secondary_cycle);
        to_sleep_loop.tv_sec=0;
        to_sleep_loop.tv_nsec=30000000000-diff_rest_time_secondary_cycle.tv_nsec;
        nanosleep(&to_sleep_loop,NULL);
    }
}

//-------------------------------------
//-  Function: Init
//-------------------------------------
rtems_task Init (rtems_task_argument ignored)
{
    pthread_t thread_ctrl;
    sigset_t alarm_sig;
    int i;

    /* Block all real time signals so they can be used for the timers.
     Note: this has to be done in main() before any threads are created
     so they all inherit the same mask. Doing it later is subject to
     race conditions */
    sigemptyset (&alarm_sig);
    for (i = SIGRTMIN; i <= SIGRTMAX; i++) {
        sigaddset (&alarm_sig, i);
    }
    sigprocmask (SIG_BLOCK, &alarm_sig, NULL);

    // init display
    displayInit(SIGRTMAX);

#ifdef RASPBERRYPI
    // Init the i2C driver
    rpi_i2c_init();

    // bus registering, this init the ports needed for the conexion
    // and register the device under /dev/i2c
    rpi_i2c_register_bus("/dev/i2c", 10000);

    // open device file
    fd_i2c = open("/dev/i2c", O_RDWR);

    // register the address of the slave to comunicate with
    ioctl(fd_i2c, I2C_SLAVE, SLAVE_ADDR);
#endif

    /* Create first thread */
    pthread_create(&thread_ctrl, NULL, controller, NULL);
    pthread_join (thread_ctrl, NULL);
    exit(0);
}

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_MAXIMUM_TASKS 1
#define CONFIGURE_MAXIMUM_SEMAPHORES 10
#define CONFIGURE_MAXIMUM_FILE_DESCRIPTORS 30
#define CONFIGURE_MAXIMUM_DIRVER 10
#define CONFIGURE_MAXIMUM_POSIX_THREADS 2
#define CONFIGURE_MAXIMUM_POSIX_TIMERS 1

#define CONFIGURE_INIT
#include <rtems/confdefs.h>
