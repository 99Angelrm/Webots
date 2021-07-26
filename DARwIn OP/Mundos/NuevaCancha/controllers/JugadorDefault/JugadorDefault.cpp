// Coded by Angel Ramirez.
// Description: State machine of default behavior player.


#include "JugadorDefault.hpp"
#include <RobotisOp2GaitManager.hpp>
#include <RobotisOp2MotionManager.hpp>
#include <RobotisOp2VisionManager.hpp>
#include <webots/Accelerometer.hpp>
#include <webots/Camera.hpp>
#include <webots/Gyro.hpp>
#include <webots/LED.hpp>
#include <webots/Motor.hpp>
#include <webots/PositionSensor.hpp>
#include <webots/Speaker.hpp>

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>

using namespace webots;
using namespace managers;
using namespace std;

static double clamp(double value, double min, double max) {
  if (min > max) {
    assert(0);
    return value;
  }
  return value < min ? min : value > max ? max : value;
}

static double minMotorPositions[NMOTORS];
static double maxMotorPositions[NMOTORS];

static const char *motorNames[NMOTORS] = {
  "ShoulderR" /*ID1 */, "ShoulderL" /*ID2 */, "ArmUpperR" /*ID3 */, "ArmUpperL" /*ID4 */, "ArmLowerR" /*ID5 */,
  "ArmLowerL" /*ID6 */, "PelvYR" /*ID7 */,    "PelvYL" /*ID8 */,    "PelvR" /*ID9 */,     "PelvL" /*ID10*/,
  "LegUpperR" /*ID11*/, "LegUpperL" /*ID12*/, "LegLowerR" /*ID13*/, "LegLowerL" /*ID14*/, "AnkleR" /*ID15*/,
  "AnkleL" /*ID16*/,    "FootR" /*ID17*/,     "FootL" /*ID18*/,     "Neck" /*ID19*/,      "Head" /*ID20*/
};

JugadorDefault::JugadorDefault() : Robot() {
  mTimeStep = getBasicTimeStep();

  mEyeLED = getLED("EyeLed");
  mHeadLED = getLED("HeadLed");
  mHeadLED->set(0x00FF00);
  mBackLedRed = getLED("BackLedRed");
  mBackLedGreen = getLED("BackLedGreen");
  mBackLedBlue = getLED("BackLedBlue");
  mCamera = getCamera("Camera");
  mCamera->enable(2 * mTimeStep);
  mAccelerometer = getAccelerometer("Accelerometer");
  mAccelerometer->enable(mTimeStep);
  mGyro = getGyro("Gyro");
  mGyro->enable(mTimeStep);
  mSpeaker = getSpeaker("Speaker");

  for (int i = 0; i < NMOTORS; i++) {
    mMotors[i] = getMotor(motorNames[i]);
    string sensorName = motorNames[i];
    sensorName.push_back('S');
    mPositionSensors[i] = getPositionSensor(sensorName);
    mPositionSensors[i]->enable(mTimeStep);
    minMotorPositions[i] = mMotors[i]->getMinPosition();
    maxMotorPositions[i] = mMotors[i]->getMaxPosition();
  }

  mMotionManager = new RobotisOp2MotionManager(this);
  mGaitManager = new RobotisOp2GaitManager(this, "config.ini");
  mVisionManager = new RobotisOp2VisionManager(mCamera->getWidth(), mCamera->getHeight(), 28, 20, 50, 45, 0, 30);
}

JugadorDefault::~JugadorDefault() {
}

void JugadorDefault::myStep() {
  int ret = step(mTimeStep);
  if (ret == -1)
    exit(EXIT_SUCCESS);
}

void JugadorDefault::wait(int ms) {
  double startTime = getTime();
  double s = (double)ms / 1000.0;
  while (s + startTime >= getTime())
    myStep();
}

// Ball detection based on the ball color using the Vision Manager
// - return: indicates if the algorithm found the ball
// - args: return the position of the ball [-1.0, 1.0]

bool JugadorDefault::getBallCenter(double &x, double &y) {
  static int width = mCamera->getWidth();
  static int height = mCamera->getHeight();

  const unsigned char *im = mCamera->getImage();
  bool find = mVisionManager->getBallCenter(x, y, im);

  if (!find) {
    x = 0.0;
    y = 0.0;
    return false;
  } else {
    x = 2.0 * x / width - 1.0;
    y = 2.0 * y / height - 1.0;
    return true;
  }
}

// function containing the main feedback loop
void JugadorDefault::run() {


  // First step to update sensors values
  myStep();

  // set eye led to green
  mEyeLED->set(0x00FF00);

  // play the hello motion
  mMotionManager->playPage(1);   // init position
  //mMotionManager->playPage(24);  // hello
  mMotionManager->playPage(9);   // walkready position
  wait(200);

  // play the motion preparing the robot to walk
  mGaitManager->start();
  mGaitManager->step(mTimeStep);

  // main loop
  double px = 0.0;
  double py = 0.0;
  int fup = 0;
  int fdown = 0;
  const double acc_tolerance = 80.0;
  const double acc_step = 20;
  enum estados{
    buscar,  
    muevete,  
    kick
  };
  enum estados estado = buscar;

  while (true) {
    double x, y, neckPosition, headPosition;
    bool ballInFieldOfView = getBallCenter(x, y);
    const double *acc = mAccelerometer->getValues();

    // count how many steps the accelerometer
    // says that the robot is down
    if (acc[1] < 512.0 - acc_tolerance)
      fup++;
    else
      fup = 0;

    if (acc[1] > 512.0 + acc_tolerance)
      fdown++;
    else
      fdown = 0;

    // the robot face is down
    if (fup > acc_step) {
      mMotionManager->playPage(1);   // init position
      mMotionManager->playPage(10);  // f_up
      mMotionManager->playPage(9);   // walkready position
      fup = 0;
    }
    // the back face is down
    else if (fdown > acc_step) {
      mMotionManager->playPage(1);   // init position
      mMotionManager->playPage(11);  // b_up
      mMotionManager->playPage(9);   // walkready position
      fdown = 0;
    }
    // if the ball is in the field of view,
    // go in the direction of the ball and kick it
    switch (estado){
    case buscar:
      ballInFieldOfView = getBallCenter(x, y);
      if(ballInFieldOfView){
        estado=muevete;
        break;
      }
      // set eye led to red
      mEyeLED->set(0xFF0000);

      // turn round
      mGaitManager->setXAmplitude(0.0);
      mGaitManager->setAAmplitude(0.3);
      mGaitManager->step(mTimeStep);

      // move the head vertically
      mGaitManager->start();
      headPosition = clamp(0.7 * sin(2.0 * getTime())+0.5, minMotorPositions[19], maxMotorPositions[19]);
      mMotors[19]->setPosition(headPosition);
      break;
    case muevete:
      ballInFieldOfView = getBallCenter(x, y);
      if(!ballInFieldOfView){
        estado=buscar;
        break;
      }
       // set eye led to blue
      mEyeLED->set(0x0000FF);

      // compute the direction of the head
      // the head move at maximum by 0.015 [rad] at each time step
      x = 0.015 * x + px;
      y = 0.015 * y + py;
      px = x;
      py = y;
      neckPosition = clamp(-x, minMotorPositions[18], maxMotorPositions[18]);
      headPosition = clamp(-y, minMotorPositions[19], maxMotorPositions[19]);

      // go forwards and turn according to the head rotation
      if (y < 0.1)  // ball far away, go quickly
        mGaitManager->setXAmplitude(1.0);
      else  // ball close, go slowly
        mGaitManager->setXAmplitude(0.5);
      mGaitManager->setAAmplitude(neckPosition);
      mGaitManager->step(mTimeStep);

      // Move head
      mMotors[18]->setPosition(neckPosition);
      mMotors[19]->setPosition(headPosition);
      if(y > 0.35)estado=kick;
      break;
    case kick:
        if(!ballInFieldOfView){
        estado=buscar;
        break;
      }
        ballInFieldOfView = getBallCenter(x, y);
        mGaitManager->stop();
        wait(500);
        // set eye led to green
        mEyeLED->set(0x00FF00);
        if (x < 0.0)
          mMotionManager->playPage(13);  // left kick
        else
          mMotionManager->playPage(12);  // right kick
        mMotionManager->playPage(9);     // walkready position
        mGaitManager->start();
        px = 0.0;
        py = 0.0;
        estado=buscar;
      break;
    default:
      break;
    }
    // step
    myStep();
  }
}