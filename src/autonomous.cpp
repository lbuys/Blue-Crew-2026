#include "auto.h"
#include "config.h"
#include "pros/motors.h"
#include "pros/rtos.hpp"
#include "lift_PID.h"





extern void toggle() {
    controller.print(0, 0, "Toggle             ");
    chassis.setPose(0, 0, 0);
    // Run each leg synchronously so the next target does not cancel this one.
        chassis.moveToPoint(0, -2, 2000,{
        .forwards = false,
        .minSpeed = 127
    }, false);
    chassis.moveToPoint(0, 5, 2000, {.minSpeed = 127}, false);
    chassis.moveToPoint(0, 0, 2000, {
        .forwards = false,
        .minSpeed = 127
    }, false);
    chassis.moveToPoint(0, 5, 2000,{.minSpeed = 127},false);
    chassis.moveToPoint(0, 0, 2000,{
        .forwards = false,
        .minSpeed = 127
    }, false);
    
}

extern void skills(){
    controller.print(0,0,"Skills            ");
    chassis.setPose(0,0,0);
    chassis.moveToPoint(0,-40,2000,{},false);
}
