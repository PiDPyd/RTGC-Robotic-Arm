//Libraries Used
#include <Wire.h>
#include <I2Cdev.h>
#include <MPU6050_6Axis_MotionApps20.h>
#include <RH_ASK.h>
#include <SPI.h>

MPU6050 mpu;


RH_ASK rf_driver(4000);

// PINS
const int scl = 5;
const int sda = 4;
const int INT_PIN = 2; //Digital
const int flex = 0; // A0
//data pin of transmitter =D12(nano)


float yaw_deg = 0, pitch_deg = 0, roll_deg = 0;

uint8_t fifoBuffer[64];

volatile bool data_status = false;



void mpu_status() {
  data_status = true;
}

void setup() {
  Serial.begin(115200);   
  Serial.println("MPU Starting...");

  Wire.begin();
  Wire.setClock(400000);

  if (!rf_driver.init()) {
    Serial.println("RF TRANSMISSION FAILED");
  } else {
    Serial.println("TRANSMISSION IS READY");
  }

  mpu.initialize();
  uint8_t devStatus = mpu.dmpInitialize();

  //Offsetting Gyro
  // mpu.setXGyroOffset(220);
  // mpu.setYGyroOffset(76);
  // mpu.setZGyroOffset(-85);

  if (devStatus == 0) {
    mpu.setDMPEnabled(true);
    attachInterrupt(digitalPinToInterrupt(INT_PIN), mpu_status, RISING);
    Serial.println("DMP ready");
  } else {
    Serial.print("DMP init failed, code: ");
    Serial.println(devStatus);
    while (1); 
  }
  
}

bool getYPR_DMP(float& yaw_deg, float& pitch_deg, float& roll_deg) {
  if (!mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) {
    return false;
  }

  Quaternion q;
  VectorFloat g;
  float ypr[3];

  mpu.dmpGetQuaternion(&q, fifoBuffer);
  mpu.dmpGetGravity(&g, &q);
  mpu.dmpGetYawPitchRoll(ypr, &q, &g);

  yaw_deg   = ypr[0] * (180.0 / PI);
  pitch_deg = ypr[1] * (180.0 / PI);
  roll_deg  = ypr[2] * (180.0 / PI);

  return true;
}

void loop() {

  uint8_t flex_value = analogRead(flex);

  if (data_status) {
    data_status = false;
    getYPR_DMP(yaw_deg, pitch_deg, roll_deg);
  }
  
  //OFFSETS 
    yaw_deg = yaw_deg -14.4;
    roll_deg = roll_deg +0.2;
    pitch_deg = pitch_deg -5.8;
    //mapping ypr from hand movement to servo angle (change this as we discussed)

  if (yaw_deg < 75 && yaw_deg > -75 && pitch_deg < 75 && pitch_deg > -75 && roll_deg < 75 && roll_deg > -75) {
    uint8_t angle_yaw = map(yaw_deg,   -75, 75, 180, 0);   //change the 0 to 180 section for setting a maximum angle per servo (as we discussed)
    uint8_t angle_pitch = map(pitch_deg, -75, 75, 0, 70);   
    uint8_t angle_roll = map(roll_deg,  -75, 75, 0, 130);  
    uint8_t DataArray[4] = { angle_yaw, angle_pitch, angle_roll, flex_value };


    if (rf_driver.send((uint8_t*)DataArray, sizeof(DataArray))){
          rf_driver.waitPacketSent();
          Serial.println("Packet sent!");
    }


    delay(26);
  }
}
