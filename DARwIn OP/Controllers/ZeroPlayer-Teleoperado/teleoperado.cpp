#include "teleoperado.hpp"
#include "ZeroPlayer.hpp"
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
#include <webots/Joystick.hpp>

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>

using namespace webots;
using namespace managers;
using namespace std;

bool firstLoop = true;
bool isWalking = false;
void ZeroPlayer::teleoperado()
{
    if (firstLoop)
    {
        cout << "-------Walk example of ROBOTIS OP2-------" << endl;
        cout << "This example illustrates Gait Manager" << endl;
        cout << "Press the space bar to start/stop walking" << endl;
        cout << "Use the arrow keys to move the robot while walking" << endl;

        // First step to update sensors values
        myStep();

        // play the hello motion
        mMotionManager->playPage(9); // init position
        wait(200);
        firstLoop = false;
        mJoystick = getJoystick();
        mJoystick->enable(mTimeStep);
        mGaitManager->setBalanceEnable(true);
    }

    // main loop
    int button = 0;

    while ((button = mJoystick->getPressedButton()) >= 0)
    {
        switch (button)
        {
        case 0:
            isWalking = !isWalking;
            wait(200);
            break;
        case 8:
            mGaitManager->stop();
            wait(200);
            mMotionManager->playPage(12); //Rigth click
            break;
        case 9:
            mGaitManager->stop();
            wait(200);
            mMotionManager->playPage(13); //Left click
            break;
        default:
            cout << button << endl;
            break;
        }
    }

    // get Joystick button
    double read1 = mJoystick->getAxisValue(0);
    double amplitude1 = read1 / -32767;
    double read2 = mJoystick->getAxisValue(1);
    double amplitude2 = read2 / -32767;
    mGaitManager->setXAmplitude(amplitude1);
    mGaitManager->setAAmplitude(amplitude2);
    // cout << read2 << endl;

    isWalking ? mGaitManager->start() : mGaitManager->stop();
    mGaitManager->step(mTimeStep);
}