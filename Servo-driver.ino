//Libraries Used
#define RH_ASK_ARDUINO_USE_TIMER2

#include <Wire.h>
#include <Servo.h>
#include <RH_ASK.h>
#include <SPI.h>

RH_ASK rf_driver(4000);

//SERVO PINS AND FLEX PINS

//write all the connections here (REMINDER)
const int nservo1 = 3; //yaw
const int nservo2 = 5; //pitch
const int nservo3 = 6; //roll
const int npaw_servo = 9; //change this to the pin we attach the last servo to (PWM PIN NEEDED)
const int flex = 0; //change this to the pin we attach the flex sensor (ANALOG PIN NEEDED)

//Global Variable Declaration

int rec_pitch = 0;
int rec_yaw = 0;
int rec_roll = 0;
int flex_val = 0; //check with this angle if its max or not
int pawClose_flex = 150; //add the maximum flex value to close the paw

Servo servo1;
Servo servo2;
Servo servo3;
Servo paw_servo;

void setup() {
  Serial.begin(115200); 

  servo1.attach(nservo1);
  servo2.attach(nservo2);
  servo3.attach(nservo3);
  paw_servo.attach(npaw_servo);

  paw_servo.write(0);
  
  if (!rf_driver.init()) {
    Serial.println("RF Receiver initialization failed!");
  } else {
    Serial.println("RF Receiver ready");
  }

}

void loop() {
  
  uint8_t Receivedarr[4];
  uint8_t buflen=sizeof(Receivedarr);
  if (rf_driver.recv((uint8_t *)Receivedarr, &buflen)) {
    Serial.println("Packet received");
    rec_yaw   = Receivedarr[0];
    rec_pitch = Receivedarr[1];
    rec_roll  = Receivedarr[2];
    flex_val = Receivedarr[3];
    servo1.write(rec_yaw);
    servo2.write(rec_pitch);
    servo3.write(rec_roll);

    Serial.println(flex_val);
    if (flex_val > pawClose_flex) {
      paw_servo.write(0);
      Serial.println("PAW CLOSEE MEOW");
    }
    else {
      paw_servo.write(90);
    }
    Serial.print("{\"pitch\":");  Serial.print(rec_pitch);
    Serial.print(",\"roll\":");   Serial.print(rec_roll);
    Serial.print(",\"yaw\":");    Serial.print(rec_yaw);
    Serial.println("}");
  }
}
