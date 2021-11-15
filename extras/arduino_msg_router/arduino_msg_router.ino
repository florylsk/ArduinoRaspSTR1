// --------------------------------------
// Includes
// --------------------------------------
#include <string.h>
#include <stdio.h>
#include <Wire.h>

// --------------------------------------
// Global Constants
// --------------------------------------
#define SLAVE_ADDR 0x8
#define MESSAGE_SIZE 8

// --------------------------------------
// Global Variables
// --------------------------------------
int count = 0;
bool msg_requested = false;
bool msg_answered = false;
char request[MESSAGE_SIZE+1];
char answer[MESSAGE_SIZE+1];

// --------------------------------------
// Function: comm_server
// --------------------------------------
int comm_server()
{
   char car_aux;
    
    if (msg_requested) {
        if (msg_answered) {
            Serial.println(answer);
        } else {
            Serial.println("MSG: ERR");
        }
        // reset global variables
        msg_requested = false;
        msg_answered = false;
        memset(request,'0', MESSAGE_SIZE);
        memset(answer,'0', MESSAGE_SIZE);
    }
    
    while (!msg_requested && Serial.available()) {
        // read one character
        car_aux =Serial.read();

          //break if there is an enter
        if (car_aux == '\n') {
            break;
        }
          //Store until the eight character
        if (count <= 8) {
            request[count] = car_aux;
        }
        count++;
    }
    // if a full message has been read
    if ((car_aux == '\n') || (count == 8)) {
        if (count == 8) {
            msg_requested = true;
            Serial.println(request);
        }
        count = 0;
    }
}


// --------------------------------------
// Function: send cmd/recv answer
// --------------------------------------
int send_recv()
{
    //Serial.println(msg_requested);
    if (msg_requested == true) {

       // send message to slave
       Wire.beginTransmission(SLAVE_ADDR);
       Wire.write(request,MESSAGE_SIZE);
       Wire.endTransmission();
      
       // wait until the command is executed
       delay (400);
      
       //request and receive the answer from the slave
       Wire.requestFrom(SLAVE_ADDR, MESSAGE_SIZE);
      
       // collect the read bytes
       int i=0;
       while (Wire.available()) {
           answer[i] = Wire.read();
           i++;
         }
      msg_answered = true;
    }
    return 0;
}

// --------------------------------------
// Function: setup
// --------------------------------------
void setup() {
 
  // Initialize I2C communications as Master
  Wire.begin();

  // Setup Serial Monitor
  Serial.begin(9600);
}

// --------------------------------------
// Function: loop
// --------------------------------------
void loop() {
    comm_server();
    send_recv();
}
