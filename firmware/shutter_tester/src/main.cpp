#include <Arduino.h>

#define INPUT_PIN  2

unsigned long start;
unsigned long stop;
volatile int rising = 0;
volatile int falling = 0;

bool get_falling_edge = true;

bool compute_speed = false;

void interupt(void);

void setup() {
  Serial.begin(115200);

  pinMode(INPUT_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(INPUT_PIN), interupt, FALLING);
}

void loop() {
  if (falling) {
    get_falling_edge = false;
    detachInterrupt(digitalPinToInterrupt(INPUT));
    start = micros();
    falling = 0;
    attachInterrupt(digitalPinToInterrupt(INPUT_PIN), interupt, RISING);
  }

  if (rising) {
    detachInterrupt(digitalPinToInterrupt(INPUT));
    stop = micros();
    rising = 0;
    compute_speed = true;
  }

  if (compute_speed) {
    detachInterrupt(digitalPinToInterrupt(INPUT));
    unsigned long t_us = stop - start;
    float t_sec = t_us / (1e6);
    float t_inverse = 1.0 / (t_sec);

    Serial.print(F("Start: "));
    Serial.print(start);
    Serial.print(F("us \nStop: "));
    Serial.print(stop);
    Serial.print(F("us \nt: "));
    Serial.print(t_us);
    Serial.print(F("us ("));
    Serial.print(t_sec);
    Serial.println(F(" seconds)"));
    Serial.print(F("1/"));
    Serial.println(t_inverse);

    compute_speed = false;
    start = 0;
    stop = 0;

    get_falling_edge = true;
    attachInterrupt(digitalPinToInterrupt(INPUT_PIN), interupt, FALLING);
  }
}

void interupt(void) {
  if(get_falling_edge) {
    falling = 1;
    rising = 0;
  } else {
    rising = 1;
    falling = 0;
  }
}