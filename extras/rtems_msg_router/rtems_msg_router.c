/********************
 *  INCLUDES
 *******************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include <rtems.h>
#include <bsp.h>
#include <bsp/i2c.h>

/********************
 *  Constants
 ********************/
#define MESSAGE_SIZE 8
#define SLAVE_ADDR 0x8

// --------------------------------------
// Function: read_message
// --------------------------------------
bool read_message(char msg[MESSAGE_SIZE+1]) 
{
   char msg_aux[MESSAGE_SIZE+1];
   int i = 0;
   // read the request
   do {
      // read one character
      char c = getchar();
      // end if received an enter
      if (c == 0x0A) { 
	      break;
      // else store if belongs to the msg
      } else if (i < MESSAGE_SIZE) {
          msg_aux[i] = c;
	      i++;
      }
   } while(1);
   // end msg and copy it out
   msg_aux[i]='\0';
   memcpy(msg, msg_aux, MESSAGE_SIZE+1);
   return true;
}

/********************
 *  Function: main
 *******************/
rtems_task Init (rtems_task_argument ignored)
{
    int file;
    char request[10];
    char answer[10];
    struct timespec sleep_time;

    //set 400 msec wait between request and answer
    sleep_time.tv_sec = 0;
    sleep_time.tv_nsec = 400000000;


    // Init the i2C driver 
    rpi_i2c_init();

    // bus registering, this init the ports needed for the conexion 
    // and register the device under /dev/i2c
    rpi_i2c_register_bus("/dev/i2c", 10000);

    // open device file
    if((file = open("/dev/i2c",O_RDWR))<0){
	    printf("ERROR AL ABRIR EL FICHERO");
    }

    // register the address of the slave to comunicate with
    if(ioctl(file,I2C_SLAVE,SLAVE_ADDR)<0){
	    printf("ERROR IOCTL\n");
	    exit(-1);
    }

    // endless loop 
    while (1) {

        //clear request and answer
        memset(request,'\0',10);
        memset(answer,'\0',10);

	    // select request to send
  	    while (!read_message(request));

        // send the message to the Arduino
        if(write(file,request,MESSAGE_SIZE)<MESSAGE_SIZE){
  	        printf("ERROR: Request SPD\n");
  	        exit (-1);
        }

	    // sleep enough to perform the operation
	    nanosleep(&sleep_time,NULL);

	    // receive the answer from the Arduino
        if(read(file,answer,MESSAGE_SIZE)<MESSAGE_SIZE){
  	        printf("ERROR: Answer SPD\n");
  	        exit(-1);
        }

	    // print the answer
	    printf ("%s\n",answer);
    }
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



