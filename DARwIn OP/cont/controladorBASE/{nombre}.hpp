
#ifndef {nombre2}_HPP
#define {nombre2}_HPP

#define NMOTORS 20

#include <webots/Robot.hpp>

namespace managers {
  class RobotisOp2MotionManager;
  class RobotisOp2GaitManager;
  class RobotisOp2VisionManager;
}  // namespace managers

namespace webots {
  class Motor;
  class LED;
  class Camera;
  class Accelerometer;
  class PositionSensor;
  class Gyro;
  class Speaker;
};  // namespace webots

class {nombre} : public webots::Robot {
public:
  {nombre}();
  virtual ~{nombre}();
  void run();

private:
  int mTimeStep;

  void myStep();
  void wait(int ms);
  bool getBallCenter(double &x, double &y);

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

  managers::RobotisOp2MotionManager *mMotionManager;
  managers::RobotisOp2GaitManager *mGaitManager;
  managers::RobotisOp2VisionManager *mVisionManager;
};

#endif
