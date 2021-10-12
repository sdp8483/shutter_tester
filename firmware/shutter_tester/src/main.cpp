#include <Arduino.h>

#define INPUT_PIN  2

volatile unsigned long edge_t_us[2];
volatile int edge_i = 0;

void interupt(void);

void setup() {
  Serial.begin(115200);

  pinMode(INPUT_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(INPUT_PIN), interupt, CHANGE);
}

void loop() {
  if (edge_i == 1) {
    detachInterrupt(digitalPinToInterrupt(INPUT_PIN));

    unsigned long t = edge_t_us[1] - edge_t_us[0];

    Serial.print(F("t0: "));
    Serial.print(edge_t_us[0]);
    Serial.print(F("us t1: "));
    Serial.print(edge_t_us[1]);
    Serial.print(F("us td: "));
    Serial.print(t);
    Serial.println(F("us"));

    edge_i = 0;

    attachInterrupt(digitalPinToInterrupt(INPUT_PIN), interupt, CHANGE);
  }
}

void interupt(void) {
  edge_t_us[edge_i] = micros();
  edge_i++;
}