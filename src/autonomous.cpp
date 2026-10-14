#include "auto.h"
#include "config.h"
#include "pros/motors.h"
#include "pros/rtos.hpp"
#include "lift_PID.h"





void toggle() {
    controller.print(0, 0, "Toggle             ");
    chassis.setPose(0, 0, 0);
    // Run each leg synchronously so the next target does not cancel this one.
    chassis.moveToPoint(0, 5, 2000, {}, false);
    chassis.moveToPoint(0, 0, 2000, {
        .forwards = false
    }, false);
    chassis.moveToPoint(0, 5, 2000,{},false);
    chassis.moveToPoint(0, 0, 2000,{
        .forwards = false
    }, false);
    
}
