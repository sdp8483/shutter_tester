#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128      // OLED display width, in pixels
#define SCREEN_HEIGHT 64      // OLED display height, in pixels

#define SHUTTER_SENSOR_PIN  2
#define SERIAL_BAUD         115200

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
#define OLED_RESET          -1      // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS      0x3C    // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

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
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/* Function Prototypes ------------------------------------------------------- */ 
void shutter_isr(void);
void shutter_init(ShutterSpeed_Handle_t *shutter);
void drawScreen(Adafruit_SSD1306 *display, ShutterSpeed_Handle_t *shutter);

void setup() {
  Serial.begin(SERIAL_BAUD);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  shutter_init(&shutter);               /* initialize shutter variable */
  
  drawScreen(&display, &shutter);                 /* draw display */

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

      drawScreen(&display, &shutter);

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

/* display measurement to SSD1306 display ------------------------------------ */
void drawScreen(Adafruit_SSD1306 *display, ShutterSpeed_Handle_t *shutter) {
  display->clearDisplay();

  display->setTextSize(1);
  display->setTextColor(SSD1306_WHITE);
  
  display->drawRect(0, 0, display->width(), 11, SSD1306_WHITE);

  display->setCursor(2, 2);   // top left corner
  display->print(F("Shutter Tester"));

  display->setCursor(0, 12);
  display->print(F("Start: "));
  display->print(shutter->start);
  display->print("us");

  display->setCursor(0, 20);
  display->print(F("Stop:  "));
  display->print(shutter->stop);
  display->print("us");

  display->setCursor(0, 28);
  display->print(F("Duration: "));
  display->print(shutter->duration_us);
  display->print("us");

  display->setCursor(0, 38);
  display->setTextSize(2);

  if (shutter->speed > 1.0) {
    display->print(F("1/"));
    display->print(shutter->speed);
  } else {
    display->print(shutter->duration_sec);
    display->print(F("sec"));
  }
  
  display->display();

}

// void testdrawchar(void) {
//   display.clearDisplay();

//   display.setTextSize(1);      // Normal 1:1 pixel scale
//   display.setTextColor(SSD1306_WHITE); // Draw white text
//   display.setCursor(0, 0);     // Start at top-left corner
//   display.cp437(true);         // Use full 256 char 'Code Page 437' font

//   // Not all the characters will fit on the display. This is normal.
//   // Library will draw what it can and the rest will be clipped.
//   for(int16_t i=0; i<256; i++) {
//     if(i == '\n') display.write(' ');
//     else          display.write(i);
//   }

//   display.display();
//   delay(2000);
// }