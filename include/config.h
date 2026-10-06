#include "main.h"
#include "lemlib/api.hpp"
#include "api.h"
#include "pros/adi.hpp"
#include "pros/optical.hpp"
#include "pros/rotation.hpp"


#pragma once

extern pros::Controller controller;


extern pros::MotorGroup leftMotors;
extern pros::MotorGroup rightMotors;
extern pros::Motor lift_11W;
extern pros::Motor lift_half;
extern pros::Motor scoring_mech;
extern pros::adi::DigitalOut scorepiston;
extern pros::Rotation liftenc;

extern pros::Imu imu;

extern pros::Distance front_sensor;
extern pros::Distance back_sensor;
extern pros::Distance left_sensor;
extern pros::Distance right_sensor;

extern pros::Rotation verticalEnc;
extern pros::Rotation horizontalEnc;
extern lemlib::TrackingWheel vertical;
extern lemlib::TrackingWheel horizontal;


extern lemlib::Drivetrain drivetrain;

extern lemlib::ControllerSettings linearController;

extern lemlib::ControllerSettings angularController;

extern lemlib::OdomSensors sensors;

extern lemlib::ExpoDriveCurve throttleCurve;

extern lemlib::ExpoDriveCurve steerCurve;

extern lemlib::Chassis chassis;
