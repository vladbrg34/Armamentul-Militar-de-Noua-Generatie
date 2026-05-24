#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Servo.h>

void processCommand(char command);
void setMotorSpeed(int speedValue);
void motorForward();
void motorBackward();
void motorLeft();
void motorRight();
void motorStop();
void turretLeft();
void turretRight();
void launcherFire();
void clearMotorInputs();
void setMotorInputs(uint8_t in1, uint8_t in2, uint8_t in3, uint8_t in4);
void timer2_init();
void handleLauncherReturn();

SoftwareSerial bluetooth(2, 3); //TX PD2, RX PD3 

const int ENA = 5; //PD5
const int ENB = 6; //PD6

const int TURRET_SERVO_PIN = 9; //PB1
const int LAUNCHER_SERVO_PIN = 10; //PB2

Servo turretServo;
Servo launcherServo;

int motorSpeed = 130; //viteza default; poate fi maxim 255

const int LEFT_MOTOR_OFFSET = -30; //offset pentru calibrarea motoarelor
const int RIGHT_MOTOR_OFFSET = 0;

int turretAngle = 90;
const int TURRET_STEP = 10;
const int TURRET_MIN = 20;
const int TURRET_MAX = 160;

const int LAUNCHER_LOCK_ANGLE = 20;
const int LAUNCHER_RELEASE_ANGLE = 90;

volatile uint16_t launcherTimerMs = 0;
volatile bool launcherReturnRequested = false;
bool launcherIsReleased = false;

void setup()
{
  DDRC |= (1 << PC0) | (1 << PC1) | (1 << PC2) | (1 << PC3);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  motorStop();
  timer2_init();
  turretServo.attach(TURRET_SERVO_PIN);
  launcherServo.attach(LAUNCHER_SERVO_PIN);
  turretServo.write(turretAngle);
  launcherServo.write(LAUNCHER_LOCK_ANGLE);
  bluetooth.begin(9600);
}

void timer2_init()
{
  cli();
  TCCR2A = 0;
  TCCR2B = 0;
  TCCR2A |= (1 << WGM21);
  OCR2A = 249;
  TIMSK2 |= (1 << OCIE2A);
  TCCR2B |= (1 << CS22);
  sei();
}

ISR(TIMER2_COMPA_vect)
{
  if (launcherTimerMs > 0) {
    launcherTimerMs--;
    if (launcherTimerMs == 0) {
      launcherReturnRequested = true;
    }
  }
}

void loop()
{
  if (bluetooth.available()) {
    char command = bluetooth.read();
    processCommand(command);
  }
  handleLauncherReturn();
}

void processCommand(char command)
{
  switch (command) {
    case 'F':
      motorForward();
      break;

    case 'B':
      motorBackward();
      break;

    case 'L':
      motorLeft();
      break;

    case 'R':
      motorRight();
      break;

    case 'S':
      motorStop();
      break;

    case 'A':
      turretLeft();
      break;

    case 'D':
      turretRight();
      break;

    case 'P':
      launcherFire();
      break;

    case '0':
      motorSpeed = 0;
      setMotorSpeed(motorSpeed);
      break;

    case '1':
      motorSpeed = 130;
      setMotorSpeed(motorSpeed);
      break;

    case '2':
      motorSpeed = 155;
      setMotorSpeed(motorSpeed);
      break;

    case '3':
      motorSpeed = 170;
      setMotorSpeed(motorSpeed);
      break;

    case '4':
      motorSpeed = 185;
      setMotorSpeed(motorSpeed);
      break;

    case '5':
      motorSpeed = 200;
      setMotorSpeed(motorSpeed);
      break;

    default:
      break;
  }
}

void clearMotorInputs()
{
  PORTC &= ~((1 << PC0) | (1 << PC1) | (1 << PC2) | (1 << PC3));
}

void setMotorInputs(uint8_t in1, uint8_t in2, uint8_t in3, uint8_t in4)
{
  clearMotorInputs();
  if (in1) PORTC |= (1 << PC0);  //IN1
  if (in2) PORTC |= (1 << PC1);  //IN2
  if (in3) PORTC |= (1 << PC2);  //IN3
  if (in4) PORTC |= (1 << PC3);  //IN4
}

void setMotorSpeed(int speedValue)
{
  speedValue = constrain(speedValue, 0, 255);
  int leftSpeed = speedValue + LEFT_MOTOR_OFFSET;
  int rightSpeed = speedValue + RIGHT_MOTOR_OFFSET;
  leftSpeed = constrain(leftSpeed, 0, 255);
  rightSpeed = constrain(rightSpeed, 0, 255);
  analogWrite(ENA, leftSpeed);
  analogWrite(ENB, rightSpeed);
}

void motorBackward()
{
  setMotorInputs(1, 0, 1, 0);
  setMotorSpeed(motorSpeed);
}

void motorForward()
{
  setMotorInputs(0, 1, 0, 1);
  setMotorSpeed(motorSpeed);
}

void motorLeft()
{
  setMotorInputs(0, 0, 1, 0);
  setMotorSpeed(motorSpeed);
}

void motorRight()
{
  setMotorInputs(1, 0, 0, 0);
  setMotorSpeed(motorSpeed);
}

void motorStop()
{
  clearMotorInputs();
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

void turretLeft()
{
  turretAngle -= TURRET_STEP;
  if (turretAngle < TURRET_MIN) {
    turretAngle = TURRET_MIN;
  }
  turretServo.write(turretAngle);
}

void turretRight()
{
  turretAngle += TURRET_STEP;
  if (turretAngle > TURRET_MAX) {
    turretAngle = TURRET_MAX;
  }
  turretServo.write(turretAngle);
}

void launcherFire()
{
  if (!launcherIsReleased) {
    launcherServo.write(LAUNCHER_RELEASE_ANGLE);
    launcherIsReleased = true;
    launcherTimerMs = 400;
    launcherReturnRequested = false;
  }
}

void handleLauncherReturn()
{
  if (launcherReturnRequested) {
    launcherReturnRequested = false;
    launcherServo.write(LAUNCHER_LOCK_ANGLE);
    launcherIsReleased = false;
  }
}