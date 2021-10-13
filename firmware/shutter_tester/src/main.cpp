#include <Arduino.h>

#define SHUTTER_SENSOR_PIN  2
#define SERIAL_BAUD         115200

/* User Typedef -------------------------------------------------------------- */
typedef enum __edge_t {
  NO_EDGE,
  FALLING_EDGE,
  RISING_EDGE,
  CALC_EDGE,
} edge_t;

typedef struct __shutterSpeed_handle_t {
  unsigned long start;
  unsigned long stop;
  edge_t edge;
  edge_t edge_to_detect;
  unsigned long duration_us;
  double duration_sec;
  double speed;
} ShutterSpeed_Handle_t;

/* Global Variables ---------------------------------------------------------- */
ShutterSpeed_Handle_t shutter;

/* Function Prototypes ------------------------------------------------------- */ 
void shutter_isr(void);
void shutter_init(ShutterSpeed_Handle_t *shutter);

void setup() {
  Serial.begin(SERIAL_BAUD);

  shutter_init(&shutter);               /* initialize shutter variable */

  pinMode(SHUTTER_SENSOR_PIN, INPUT);   /* shutter sensor pin is input */

  attachInterrupt(digitalPinToInterrupt(SHUTTER_SENSOR_PIN), shutter_isr, FALLING);
} // end setup()

void loop() {
  switch (shutter.edge) {
    case FALLING_EDGE:
      detachInterrupt(digitalPinToInterrupt(SHUTTER_SENSOR_PIN));
      shutter.edge = NO_EDGE;
      shutter.edge_to_detect = RISING_EDGE;
      shutter.start = micros();
      attachInterrupt(digitalPinToInterrupt(SHUTTER_SENSOR_PIN), shutter_isr, RISING);
      break;
    case RISING_EDGE:
      detachInterrupt(digitalPinToInterrupt(SHUTTER_SENSOR_PIN));
      shutter.edge = CALC_EDGE;
      shutter.stop = micros();
      
      break;
    case CALC_EDGE:
      shutter.duration_us = shutter.stop - shutter.start;
      shutter.duration_sec = shutter.duration_us / 1e6;
      shutter.speed = 1 / shutter.duration_sec;

      Serial.print(F("Start: "));
      Serial.print(shutter.start);
      Serial.print(F("us \nStop: "));
      Serial.print(shutter.stop);
      Serial.print(F("us \nDuration: "));
      Serial.print(shutter.duration_us);
      Serial.print(F("us \nSpeed: 1/"));
      Serial.println(shutter.speed);
      Serial.println();

      shutter_init(&shutter);               /* reset shutter variable */

      attachInterrupt(digitalPinToInterrupt(SHUTTER_SENSOR_PIN), shutter_isr, FALLING);

      break;
    
    case NO_EDGE:

      break;

    default:
      shutter.edge = NO_EDGE;
      break;
  }

} // end loop()

void shutter_isr(void) {
  shutter.edge = shutter.edge_to_detect;
} // end shutter_isr()

/* Set/Reset ShutterSpeed_Handle --------------------------------------------- */
void shutter_init(ShutterSpeed_Handle_t *shutter) {
  shutter->start = 0;
  shutter->stop = 0;
  shutter->duration_us = 0;
  shutter->duration_sec = 0;
  shutter->speed = 0;
  shutter->edge = NO_EDGE;
  shutter->edge_to_detect = FALLING_EDGE;
} // end shutter_init()