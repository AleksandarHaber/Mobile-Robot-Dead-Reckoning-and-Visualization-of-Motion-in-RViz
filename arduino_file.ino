/* First version of the Arduino-ROS code for
  1) Receiving control actions from ROS. We receive  (desired) velocity commands
  for left and right motors.  
  2) Sending back encoder pulses. We send back detected encoder pulses for
  left and right encoders.
  3) Controlling the motors on the basis of the received velocity commands
*/
/////////////////////////////////////////////////////////////////////
//              Start: Header files
/////////////////////////////////////////////////////////////////////
// Arduino-ROS Library - Rosserial Arduino library - you need to install this library in Arduino
#include <ros.h>
// we will be receiving integer messages (velocities) and we will send back
// integer messages (encoder pulses) consequently, we need ROS Int32 data structure
#include <std_msgs/Int32.h>

/////////////////////////////////////////////////////////////////////
//              End: Header files
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
//              Start: Arduino Pins
/////////////////////////////////////////////////////////////////////
// encoder pins, note that 2 and 3 are interrupt pins. Encoders should be connected
// to the interrupt pins, otherwise they might not work!
// left encoder
int encoderPinLeft=2;
// right encoder
int encoderPinRight=3;

// right motor pins
int IN3 = 9;
int IN4 = 10;
int ENB = 11;
// left motor pins
int ENA = 5;
int IN1 = 6;
int IN2 = 7;
/////////////////////////////////////////////////////////////////////
//              End: Arduino Pins
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
//              Start: Other Arduino global variables and global objects
/////////////////////////////////////////////////////////////////////
// these two variables count the total number of encoder pulses
// left encoder pulses
volatile unsigned long totalPulsesLeft = 0;  
// right encoder pulses
volatile unsigned long totalPulsesRight = 0;  

// motor velocities - these variables are set by ROS
// left motor
int motorVelocityLeft=0; // 0 to 255

// right motor
int motorVelocityRight=0;

/////////////////////////////////////////////////////////////////////
//              End: Other Arduino global variables and global objects
/////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////
//              Start: ROS stuff- variables, functions, objects, etc.
/////////////////////////////////////////////////////////////////////

//This is a handle of the ROS node
ros::NodeHandle  nh;

// this is the callback function for the left motor,
// it simply sets the velocity of the left motor on the basis of the received ROS message
void callBackFunctionMotorLeft(const std_msgs::Int32 &motorVelocityLeftROS){
  motorVelocityLeft=motorVelocityLeftROS.data;  
}
// this is the callback function for the right motor,
// it simply sets the velocity of the right motor on the basis of the received ROS message
void callBackFunctionMotorRight(const std_msgs::Int32 &motorVelocityRightROS){
  motorVelocityRight=motorVelocityRightROS.data;  
}

// publishers for the encoder pulses
// left encoder publisher
std_msgs::Int32 leftEncoderROS;
ros::Publisher leftEncoderROSPublisher("left_encoder_pulses", &leftEncoderROS);
// right encoder publisher
std_msgs::Int32 rightEncoderROS;
ros::Publisher rightEncoderROSPublisher("right_encoder_pulses", &rightEncoderROS);

//Subscribers for left and right motor velocities
// left motor
ros::Subscriber<std_msgs::Int32> leftMotorROSSubscriber("left_motor_velocity",&callBackFunctionMotorLeft);  
// right motor
ros::Subscriber<std_msgs::Int32> rightMotorROSSubscriber("right_motor_velocity",&callBackFunctionMotorRight);  
/////////////////////////////////////////////////////////////////////
//              End: ROS stuff- variables, objects, etc.
/////////////////////////////////////////////////////////////////////

void setup() {
/////////////////////////////////////////////////////////////////////
//              Start: Setup Arduino stuff
/////////////////////////////////////////////////////////////////////
  // set the encoder pins
  pinMode(encoderPinLeft, INPUT);
  pinMode(encoderPinRight, INPUT);
  // attach the interrupts for tracking the pulses
  attachInterrupt(digitalPinToInterrupt(encoderPinLeft), interruptFunctionLeft, RISING);
  attachInterrupt(digitalPinToInterrupt(encoderPinRight), interruptFunctionRight, RISING);
  // motors
  // left motor
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  // right motor
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);
 
  // Set all motors to OFF
  // left
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  // right
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
/////////////////////////////////////////////////////////////////////
//              End: Setup Arduino stuff
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
//              Start: Setup ROS stuff
/////////////////////////////////////////////////////////////////////

  // here, we set the serial communication parameters via Bluetooth
  //nh.getHardware()->setPort(&Serial1);
  nh.getHardware()->setBaud(9600);

      // here we initialize the ROS node
  nh.initNode();

  // Publishers
  nh.advertise(leftEncoderROSPublisher);
  nh.advertise(rightEncoderROSPublisher);

  // Subsribers
  nh.subscribe(leftMotorROSSubscriber);
  nh.subscribe(rightMotorROSSubscriber);

/////////////////////////////////////////////////////////////////////
//              End: Setup ROS stuff
/////////////////////////////////////////////////////////////////////
}



void loop() {

   nh.spinOnce();
  // Set the speed of the motor (PWM signals) from 0 to 255
  // the variables motorVelocityLeft and motorVelocityRight
  // are filled on the basis of the messages received from ROS
  analogWrite(ENA, motorVelocityLeft);
  analogWrite(ENB, motorVelocityRight);

  // Set the direction and turn ON
  // left motor
  digitalWrite(IN1,LOW);
  digitalWrite(IN2, HIGH);
  // Set the direction and turn ON
  // right motor
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  nh.spinOnce();

  // send back the encoder readings
  leftEncoderROS.data = totalPulsesLeft;
  rightEncoderROS.data = totalPulsesRight;
  leftEncoderROSPublisher.publish(&leftEncoderROS);
  rightEncoderROSPublisher.publish(&rightEncoderROS);
  nh.spinOnce();
 
  delay(20);
}

// interrup function left encoder
void interruptFunctionLeft(){
         // increment the total number of pulses
        totalPulsesLeft = totalPulsesLeft + 1;
           
}
// interrup function right encoder
void interruptFunctionRight(){
         // increment the total number of pulses
        totalPulsesRight = totalPulsesRight + 1;
}

