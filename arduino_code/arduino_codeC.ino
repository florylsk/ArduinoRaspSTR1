// --------------------------------------
// Include files
// --------------------------------------
#include <string.h>
#include <stdio.h>
#include <Wire.h>
#include "SevSeg.h"
// --------------------------------------
// Global Constants
// --------------------------------------
#define SLAVE_ADDR 0x8
#define MESSAGE_SIZE 8
#define SWITCH_PIN_NEGATIVE_SLOPE 8
#define SWITCH_PIN_POSITIVE_SLOPE 9
#define LED_SPEED 10
#define LED_MIXER 11
#define LED_BRAKE 12
#define LED_GAS 13
#define LED_SPOTLIGHTS 7
#define A 2
#define B 3
#define C 4
#define D 5
#define VALIDATION_BUTTON_PIN 6


// --------------------------------------
// Global Variables
// --------------------------------------
double speed = 55.5;
bool request_received = false;
bool answer_requested = false;
char request[MESSAGE_SIZE + 1];
char answer[MESSAGE_SIZE + 1];
short slope = 0; //-1->bajada,0->llano,1->subida
double acc_slope = 0;
double acc_manual =0;
short mixer_state = 0;
short gas_state = 0;
short brake_state = 0;
int luminosity=0;
short spotlight_state = 0;
int const LDR_PIN=A0;
int const DISTANCE_PIN=A1;
unsigned long int end_rest_time_loop=0;
unsigned long int diff_rest_time_loop=0;
unsigned long int start_rest_time_loop=0;
int distance_potentiometer;
int mode=0; //0=seleccion de distancia,1=aproximacion, 2=parada
SevSeg sevseg;
int validated_distance;
int real_distance;
// --------------------------------------
// Handler function: receiveEvent
// --------------------------------------
void receiveEvent(int num)
{
  char aux_str[MESSAGE_SIZE + 1];
  int i = 0;

  // read message char by char
  for (int j = 0; j < num; j++) {
    char c = Wire.read();
    if (i < MESSAGE_SIZE) {
      aux_str[i] = c;
      i++;
    }
  }
  aux_str[i] = '\0';

  // if message is correct, load it
  if ((num == MESSAGE_SIZE) && (!request_received)) {
    memcpy(request, aux_str, MESSAGE_SIZE + 1);
    request_received = true;
  }
}

// --------------------------------------
// Handler function: requestEvent
// --------------------------------------
void requestEvent()
{
  // if there is an answer send it, else error
  if (answer_requested) {
    Wire.write(answer, MESSAGE_SIZE);
    memset(answer, '\0', MESSAGE_SIZE + 1);
  } else {
    Wire.write("MSG: ERR", MESSAGE_SIZE);
  }

  // set answer empty
  request_received = false;
  answer_requested = false;
  memset(request, '\0', MESSAGE_SIZE + 1);
  memset(answer, '0', MESSAGE_SIZE);
}

// TASK 1
void show_gas() {
  if (gas_state == 1) {
    digitalWrite(LED_GAS, HIGH);
  }
  else if (gas_state == 0) {
    digitalWrite(LED_GAS, LOW);
  }

}

// TASK 2
void show_brake() {
  if (brake_state == 1) {
    digitalWrite(LED_BRAKE, HIGH);
  }
  else if (brake_state == 0) {
    digitalWrite(LED_BRAKE, LOW);
  }

}

// TASK 3
void show_mixer() {
  if (mixer_state == 0) {
    digitalWrite(LED_MIXER, LOW);
  } else {
    digitalWrite(LED_MIXER, HIGH);
  }

}

// TASK 4
void get_slope() {

  int reading_switch_bajada = digitalRead(SWITCH_PIN_NEGATIVE_SLOPE);
  int reading_switch_pendiente = digitalRead(SWITCH_PIN_POSITIVE_SLOPE);
  if (reading_switch_bajada == HIGH && reading_switch_pendiente == LOW ) {
    acc_slope=0.25;
    slope = -1;
  }
  else if (reading_switch_bajada == LOW && reading_switch_pendiente == HIGH ) {
    // slope is positive
    acc_slope=-0.25;
    slope = 1;
  }
  else if (reading_switch_bajada == LOW && reading_switch_pendiente == LOW ) {
    acc_slope=0;
    slope = 0;
  }
  return;
}

// TASK 5
void show_speed() {
  double v0 = speed;

  double dt = 0.2; //hiperperiod
  double vf = v0 + (acc_manual * dt)+(acc_slope*dt);
  //sets new speed as final value
  speed = vf;
  if (speed < 40) {
    analogWrite(LED_SPEED, 0);
  }
  else if (speed >= 40 && speed <= 70 ) {
    int diff = 70 - speed;
    int brightness_speed;
    if (diff >= 0 && diff <= 10) {
      brightness_speed = 255;
    }
    else if (diff > 10 && diff <= 20) {
      brightness_speed = 170;
    }
    else if (diff > 20 && diff <= 30) {
      brightness_speed = 100;
    }
    else {
      brightness_speed = 50;
    }

    analogWrite(LED_SPEED, brightness_speed);
  }
  else {
    analogWrite(LED_SPEED, 255);
  }
  return;
}




// TASK 6
void communication_server()
{
  // while there is enough data for a request
  if ( (request_received)) {
    if (0 == strcmp("GAS: SET", request)) {
      acc_manual=0;
      acc_manual+= 0.5/5;
      brake_state=0;
      gas_state = 1;
      sprintf(answer, "GAS:  OK");
      memset(request, '\0', MESSAGE_SIZE + 1);
      request_received = false;
      answer_requested = true;
    }
    else if (0 == strcmp("GAS: CLR", request)) {
      acc_manual=0;
      gas_state = 0;
      sprintf(answer, "GAS:  OK");
      memset(request, '\0', MESSAGE_SIZE + 1);
      request_received = false;
      answer_requested = true;
    }
    else if (0 == strcmp("BRK: SET", request)) {
      acc_manual=0;
      acc_manual= acc_manual - 0.5/5;
      gas_state=0;
      brake_state = 1;
      sprintf(answer, "BRK:  OK");
      memset(request, '\0', MESSAGE_SIZE + 1);
      request_received = false;
      answer_requested = true;
    }
    else if (0 == strcmp("BRK: CLR", request)) {
      acc_manual=0;
      brake_state = 0;
      sprintf(answer, "BRK:  OK");
      memset(request, '\0', MESSAGE_SIZE + 1);
      request_received = false;
      answer_requested = true;
    }
    else if (0 == strcmp("MIX: SET", request)) {
      mixer_state = 1;
      sprintf(answer, "MIX:  OK");
      memset(request, '\0', MESSAGE_SIZE + 1);
      request_received = false;
      answer_requested = true;
    }
    else if (0 == strcmp("MIX: CLR", request)) {
      mixer_state = 0;
      sprintf(answer, "MIX:  OK");
      memset(request, '\0', MESSAGE_SIZE + 1);
      request_received = false;
      answer_requested = true;
    }
    else if (0 == strcmp("SPD: REQ", request)) {
      char num_str[5];
      dtostrf(speed, 4, 1, num_str);
      sprintf(answer, "SPD:%s", num_str);

      memset(request, '\0', MESSAGE_SIZE + 1);
      request_received = false;
      answer_requested = true;
    }
    else if (0 == strcmp("SLP: REQ", request)) {
      char slp_str[5];
      switch (slope) {
        case -1:
          strcpy(slp_str,"DOWN");
          break;
        case 1:
          strcpy(slp_str,"  UP");
          break;
        default:
          strcpy(slp_str,"FLAT");
      }
      sprintf(answer, "SLP:%s", slp_str);

      memset(request, '\0', MESSAGE_SIZE + 1);
      request_received = false;
      answer_requested = true;
    }
    else if (0 == strcmp("LIT: REQ", request)){
      double lum_percentage=luminosity/10;
      char lum_str[5];
      dtostrf(lum_percentage,4,1,lum_str);
      sprintf(answer,"LIT:%s",lum_str);

      memset(request, '\0', MESSAGE_SIZE + 1);
      request_received = false;
      answer_requested = true;
    }
    else if (0 == strcmp("LAM: SET", request)){
      spotlight_state = 1;
      sprintf(answer, "LAM:  OK");
      memset(request, '\0', MESSAGE_SIZE + 1);
      request_received = false;
      answer_requested = true;
    }
    else if (0 == strcmp("LAM: CLR", request)){
      spotlight_state = 0;
      sprintf(answer, "LAM:  OK");
      memset(request, '\0', MESSAGE_SIZE + 1);
      request_received = false;
      answer_requested = true;
    }
    else if (0== strcmp("STP: REQ", request)){
        if (speed>0){
            sprintf(answer, "STP:  GO");
            memset(request, '\0', MESSAGE_SIZE + 1);
            request_received = false;
            answer_requested = true;
        }
        else{
            sprintf(answer, "STP:STOP");
            memset(request, '\0', MESSAGE_SIZE + 1);
            request_received = false;
            answer_requested = true;
        }

    }
    else if (0== strcmp("DS:  REQ", request)){
        sprintf(answer, "DS:%s", real_distance);
        memset(request, '\0', MESSAGE_SIZE + 1);
        request_received = false;
        answer_requested = true;

    }
    else {
      sprintf(answer, "MSG: ERR");

      memset(request, '\0', MESSAGE_SIZE + 1);
      request_received = false;
      answer_requested = true;
    }

  }
  return;
}

void read_luminosity(){
  luminosity= analogRead(LDR_PIN);
}

void show_luminosity(){
  if (spotlight_state==1){
    digitalWrite(LED_SPOTLIGHTS,HIGH);
  }
  else{
    digitalWrite(LED_SPOTLIGHTS,LOW);
  }
}

void read_distance(){
    distance_potentiometer=analogRead(DISTANCE_PIN);



}
void to_BCD(int toShow)
{
    if (toShow == 0) //write 0000
    {
        digitalWrite(A, LOW);
        digitalWrite(B, LOW);
        digitalWrite(C, LOW);
        digitalWrite(D, LOW);
    }

    if (toShow == 1) //write 0001
    {
        digitalWrite(A, HIGH);
        digitalWrite(B, LOW);
        digitalWrite(C, LOW);
        digitalWrite(D, LOW);
    }

    if (toShow == 2) //write 0010
    {
        digitalWrite(A, LOW);
        digitalWrite(B, HIGH);
        digitalWrite(C, LOW);
        digitalWrite(D, LOW);
    }

    if (toShow == 3) //write 0011
    {
        digitalWrite(A, HIGH);
        digitalWrite(B, HIGH);
        digitalWrite(C, LOW);
        digitalWrite(D, LOW);
    }

    if (toShow == 4) //write 0100
    {
        digitalWrite(A, LOW);
        digitalWrite(B, LOW);
        digitalWrite(C, HIGH);
        digitalWrite(D, LOW);
    }

    if (toShow == 5) //write 0101
    {
        digitalWrite(A, HIGH);
        digitalWrite(B, LOW);
        digitalWrite(C, HIGH);
        digitalWrite(D, LOW);
    }

    if (toShow == 6) //write 0110
    {
        digitalWrite(A, LOW);
        digitalWrite(B, HIGH);
        digitalWrite(C, HIGH);
        digitalWrite(D, LOW);
    }

    if (toShow == 7) //write 0111
    {
        digitalWrite(A, HIGH);
        digitalWrite(B, HIGH);
        digitalWrite(C, HIGH);
        digitalWrite(D, LOW);
    }

    if (toShow == 8) //write 1000
    {
        digitalWrite(A, LOW);
        digitalWrite(B, LOW);
        digitalWrite(C, LOW);
        digitalWrite(D, HIGH);
    }

    if (toShow == 9) //write 1001
    {
        digitalWrite(A, HIGH);
        digitalWrite(B, LOW);
        digitalWrite(C, LOW);
        digitalWrite(D, HIGH);
    }
}
void show_distance() {
    if (mode == 0) {
    int decenas_miles_metros = distance_potentiometer / 10000;
    to_BCD(decenas_miles_metros);
    }
    if (mode==1){
        real_distance=validated_distance-(speed*0.2)+((1/2)*(acc_manual+acc_slope)*0.04);
        to_BCD(real_distance/10000);
        if(real_distance<=0 && speed<=10){
            real_distance=0;
            to_BCD(real_distance);
            mode=2;
        }
        else if (real_distance<=0 && speed>10){
            mode=0;
        }
    }
}
void validation_distance(){
    int buton_validation_value=digitalRead(VALIDATION_BUTTON_PIN);
    if (buton_validation_value==HIGH){
            validated_distance = distance_potentiometer;
            mode = 1;

    }
}
void set_speed_zero(){
    speed=0;
}

void read_EndOfStop(){
    int buton_validation_value=digitalRead(VALIDATION_BUTTON_PIN);
    if (buton_validation_value==HIGH){
        mode = 0;

    }

}
//
// --------------------------------------
// Function: setup
// --------------------------------------
void setup()
{
  // Initialize I2C communications as Slave
  Wire.begin(SLAVE_ADDR);

  // Function to run when data requested from master
  Wire.onRequest(requestEvent);

  // Function to run when data received from master
  Wire.onReceive(receiveEvent);
  pinMode(SWITCH_PIN_NEGATIVE_SLOPE, INPUT);
  pinMode(SWITCH_PIN_POSITIVE_SLOPE, INPUT);
  pinMode(LED_GAS, OUTPUT);
  pinMode(LED_BRAKE, OUTPUT);
  pinMode(LED_MIXER, OUTPUT);
  pinMode(LED_SPEED, OUTPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(LED_SPOTLIGHTS,OUTPUT);
  pinMode(DISTANCE_PIN,INPUT);
  pinMode(A, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(C, OUTPUT);
  pinMode(D, OUTPUT);
  pinMode(VALIDATION_BUTTON_PIN,INPUT);
}

// --------------------------------------
// Function: loop
// --------------------------------------
void loop()
{
    if (mode==0) {
        start_rest_time_loop = millis();
        show_gas();
        show_brake();
        show_mixer();
        show_speed();
        get_slope();
        read_luminosity();
        show_luminosity();
        read_distance();
        show_distance();
        validation_distance();
        communication_server();
        //SLEEP
        end_rest_time_loop = millis();
        diff_rest_time_loop = end_rest_time_loop - start_rest_time_loop;
        delay(200 - diff_rest_time_loop);
    }
    else if (mode==1){
        start_rest_time_loop = millis();
        show_gas();
        show_brake();
        show_mixer();
        show_speed();
        get_slope();
        read_luminosity();
        show_luminosity();
        show_distance();
        communication_server();

        //SLEEP
        diff_rest_time_loop = end_rest_time_loop - start_rest_time_loop;
        delay(200 - diff_rest_time_loop);



    }



    else{
        start_rest_time_loop = millis();
        show_gas();
        show_brake();
        show_mixer();
        show_speed();
        get_slope();
        read_luminosity();
        show_luminosity();
        read_EndOfStop();
        communication_server();
        // SLEEP
        end_rest_time_loop = millis();
        diff_rest_time_loop = end_rest_time_loop - start_rest_time_loop;
        delay(200 - diff_rest_time_loop);
    }
}
