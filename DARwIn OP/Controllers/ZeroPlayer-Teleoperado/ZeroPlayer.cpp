#include "ZeroPlayer.hpp"
#include "teleoperado.hpp"
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

double clamp(double value, double min, double max)
{
  if (min > max)
  {
    assert(0);
    return value;
  }
  return value < min ? min : value > max ? max
                                         : value;
}
int ZeroPlayer::safe()
{
  const double *acc = mAccelerometer->getValues();
  if (acc[1] < 512.0 - acc_tolerance)
    fup++;
  else
    fup = 0;

  if (acc[1] > 512.0 + acc_tolerance)
    fdown++;
  else
    fdown = 0;
  if (fup > acc_step || fdown > acc_step) return 0;
  return 1;
}
static const char *motorNames[NMOTORS] = {
    "ShoulderR" /*ID1 */, "ShoulderL" /*ID2 */, "ArmUpperR" /*ID3 */, "ArmUpperL" /*ID4 */, "ArmLowerR" /*ID5 */,
    "ArmLowerL" /*ID6 */, "PelvYR" /*ID7 */, "PelvYL" /*ID8 */, "PelvR" /*ID9 */, "PelvL" /*ID10*/,
    "LegUpperR" /*ID11*/, "LegUpperL" /*ID12*/, "LegLowerR" /*ID13*/, "LegLowerL" /*ID14*/, "AnkleR" /*ID15*/,
    "AnkleL" /*ID16*/, "FootR" /*ID17*/, "FootL" /*ID18*/, "Neck" /*ID19*/, "Head" /*ID20*/
};

ZeroPlayer::ZeroPlayer() : Robot()
{
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

  for (int i = 0; i < NMOTORS; i++)
  {
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

ZeroPlayer::~ZeroPlayer()
{
}

void ZeroPlayer::myStep()
{
  int ret = step(mTimeStep);
  if (ret == -1)
    exit(EXIT_SUCCESS);
}

void ZeroPlayer::wait(int ms)
{
  double startTime = getTime();
  double s = (double)ms / 1000.0;
  while (s + startTime >= getTime())
    myStep();
}

// Ball detection based on the ball color using the Vision Manager
// - return: indicates if the algorithm found the ball
// - args: return the position of the ball [-1.0, 1.0]

bool ZeroPlayer::getBallCenter(double &x, double &y)
{
  static int width = mCamera->getWidth();
  static int height = mCamera->getHeight();

  const unsigned char *im = mCamera->getImage();
  bool find = mVisionManager->getBallCenter(x, y, im);

  if (!find)
  {
    x = 0.0;
    y = 0.0;
    return false;
  }
  else
  {
    x = 2.0 * x / width - 1.0;
    y = 2.0 * y / height - 1.0;
    return true;
  }
}

// function containing the main feedback loop
void ZeroPlayer::run()
{
  enum estados
  {
    init,
    StandUp,
    Play
  };
  enum estados state = init;
  const char *states[]{"init",
                       "StandUp",
                       "Play"};

  while (true)
  {
    switch (state)
    {
    case init:
      // First step to update sensors values
      myStep();

      // set eye led to green
      mEyeLED->set(0x00FF00);

      // play the hello motion
      mMotionManager->playPage(1); // init position
      //mMotionManager->playPage(24);  // hello
      mMotionManager->playPage(9); // walkready position
      wait(200);
      // play the motion preparing the robot to walk
      mGaitManager->start();
      mGaitManager->step(mTimeStep);
      state = Play;
      break;
    case StandUp:
      if (fup > acc_step)
      {
        mMotionManager->playPage(1);  // init position
        mMotionManager->playPage(10); // f_up
        mMotionManager->playPage(9);  // walkready position
        fup = 0;
      }
      // the back face is down
      else if (fdown > acc_step)
      {
        mMotionManager->playPage(1);  // init position
        mMotionManager->playPage(11); // b_up
        mMotionManager->playPage(9);  // walkready position
        fdown = 0;
      }
      state = Play;
      break;
    case Play:
      if (safe()==0)state=StandUp; //0 not safe
      teleoperado(); //
      break;

    default:
      break;
    }
    cout << states[state] << endl;
    myStep();
  }
}
