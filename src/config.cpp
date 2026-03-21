#include "auto.h"
#include "lemlib/chassis/trackingWheel.hpp"
#include "config.h"
#include "pros/distance.hpp"


// Controller
pros::Controller controller(pros::E_CONTROLLER_MASTER);

// Motors and Pistons


// Motor groups
pros::MotorGroup leftMotors({-18,-20, -17}, pros::MotorGearset::blue);
pros::MotorGroup rightMotors({16, 14,13}, pros::MotorGearset::blue);

// Sensors
pros::Imu imu(19);
pros::Optical optical(10);
pros::Distance front_sensor(20);
pros::Distance back_sensor(21);
pros::Distance left_sensor(22);
pros::Distance right_sensor(23);

// Tracking Wheels
// pros::Rotation horizontalEnc(11);
pros::Rotation verticalEnc(15);
// lemlib::TrackingWheel horizontal(&horizontalEnc, lemlib::Omniwheel::NEW_275, -6.5);
lemlib::TrackingWheel vertical(&verticalEnc, lemlib::Omniwheel::NEW_2, .421875);

// Drivetrain Settings
lemlib::Drivetrain drivetrain(&leftMotors, // left motor groups
                              &rightMotors, // right motor group
                              10.344, // track width
                              lemlib::Omniwheel::NEW_325, // using new 3.25" omnis
                              480, // drivetrain rpm
                              5 // horizontal drift is 2. If we had traction wheels, it would have been 8
);

// Lateral Motion Controller
lemlib::ControllerSettings linearController(3.55, // proportional gain (kP)
                                            0, // integral gain (kI)
                                            .5, // derivative gain (kD)
                                            3, // anti windup
                                            0.5, // small error range, in inches
                                            100, // small error range timeout, in milliseconds
                                            2, // large error range, in inches
                                            500, // large error range timeout, in milliseconds
                                            115 // maximum acceleration (slew)    
);

// Angular Motion Controller
lemlib::ControllerSettings angularController(.8, // proportional gain (kP)
                                             0, // integral gain (kI)
                                             .12, // derivative gain (kD)
                                             3, // anti windup
                                             2, // small error range, in degrees
                                             90, // small error range timeout, in milliseconds
                                             3, // large error range, in degrees
                                             500, // large error range timeout, in milliseconds
                                             0 // maximum acceleration (slew)
);

// Sensors For Odometry
lemlib::OdomSensors sensors(&vertical, // vertical tracking wheel
                            nullptr,
                            nullptr, // horizontal tracking wheel
                            nullptr,
                            &imu // inertial sensor
);

// input curve for throttle input during driver control
lemlib::ExpoDriveCurve throttleCurve(3, // joystick deadband out of 127
                                     10, // minimum output where drivetrain will move out of 127
                                     1.019 // expo curve gain
);
 
// input curve for steer input during driver control
lemlib::ExpoDriveCurve steerCurve(3, // joystick deadband out of 127
                                  10, // minimum output where drivetrain will move out of 127
                                  1.019 // expo curve gain
);

// create the chassis
lemlib::Chassis chassis(drivetrain, linearController, angularController, sensors, &throttleCurve, &steerCurve);