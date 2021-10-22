
#ifndef ZEROPLAYER_HPP
#define ZEROPLAYER_HPP

#define NMOTORS 20

#include <webots/Robot.hpp>
#include "teleoperado.hpp"

namespace managers
{
  class RobotisOp2MotionManager;
  class RobotisOp2GaitManager;
  class RobotisOp2VisionManager;
} // namespace managers

namespace webots
{
  class Motor;
  class LED;
  class Camera;
  class Accelerometer;
  class PositionSensor;
  class Gyro;
  class Speaker;
}; // namespace webots

class ZeroPlayer : public webots::Robot
{
public:
  ZeroPlayer();
  virtual ~ZeroPlayer();
  void run();
  int safe();
  void teleoperado();

private:
  int mTimeStep;

  void myStep();
  void wait(int ms);
  bool getBallCenter(double &x, double &y);

  double minMotorPositions[NMOTORS];
  double maxMotorPositions[NMOTORS];
  int fup = 0;
  int fdown = 0;
  const double acc_tolerance = 80.0;
  const double acc_step = 20;

  webots::Motor *mMotors[NMOTORS];
  webots::PositionSensor *mPositionSensors[NMOTORS];
  webots::LED *mEyeLED;
  webots::LED *mHeadLED;
  webots::LED *mBackLedRed;
  webots::LED *mBackLedGreen;
  webots::LED *mBackLedBlue;
  webots::Camera *mCamera;
  webots::Accelerometer *mAccelerometer;
  webots::Gyro *mGyro;
  webots::Speaker *mSpeaker;
  webots::Joystick *mJoystick;

  managers::RobotisOp2MotionManager *mMotionManager;
  managers::RobotisOp2GaitManager *mGaitManager;
  managers::RobotisOp2VisionManager *mVisionManager;
};
double clamp(double value, double min, double max);
#endif
