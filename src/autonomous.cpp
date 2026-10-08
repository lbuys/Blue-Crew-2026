#include "auto.h"
#include "config.h"
#include "pros/motors.h"
#include "pros/rtos.hpp"
#include "lift_PID.h"





extern void toggle(){
    controller.print(0, 0, "Toggle             ");
    chassis.setPose(0, 0, 0);
    //Rotate Toggle
    chassis.moveToPoint(0,2,2000);
    chassis.moveToPoint(0,0,2000, {.forwards = false});
    chassis.moveToPoint(0,2,2000);
    chassis.moveToPoint(0,0,2000, {.forwards = false});
    }
