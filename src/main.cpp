#include "main.h"
#include "config.h"
#include "auto.h"
#include "lemlib/api.hpp"
#include "reset.h"



/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */

void lvgl_task() {
    while (true) {
        lv_timer_handler(); // Handle LVGL tasks
        pros::delay(5); // Small delay to prevent CPU hogging
    }
}
void intake_task(){
    while (true) {

    }
}
void lift_PID_Task() {
    while (true) {
        lift_PID(); // Call the lift PID control function
    }}

void scorer_roller_task() {
    while (true) {
        scoring_roller(); // Call the scoring roller function
    }
}
void lift_movement_up_task() {
    while (true) {
        lift_movement_up(); // Call the lift movement up function
    }
}
void lift_movement_down_task() {
    while (true) {
        lift_movement_down(); // Call the lift movement down function
    }
}
void initialize() {
    chassis.calibrate(); // calibrate sensors
    lvgl_initialize();
    //get_starting_position(); // set starting position based on distance sensors
    pros::Task lvgl_task(lvgl_task);
    pros::Task intake_task(intake_task);
    pros::Task lift_PID_Task(lift_PID_Task);
    pros::Task scorer_roller_task(scorer_roller_task);
    pros::Task lift_movement_up_task(lift_movement_up_task);
    pros::Task lift_movement_down_task(lift_movement_down_task);
    

    // thread to for brain screen and position logging
    // pros::lcd::initialize(); // initialize brain screen
    // pros::Task screenTask([&]() {
    //     while (true) {
    //         // print robot location to the brain screen
    //         pros::lcd::print(0, "X: %f", chassis.getPose().x); // x
    //         pros::lcd::print(1, "Y: %f", chassis.getPose().y); // y
    //         pros::lcd::print(2, "Theta: %f", chassis.getPose().theta); // heading
    //         // log position telemetry
    //         lemlib::telemetrySink()->info("Chassis pose: {}", chassis.getPose());
    //         // delay to save resources
    //         pros::delay(50);
    //     }
    // });
}

// get a path used for pure pursuit
// this needs to be put outside a function
//ASSET(path_txt); // '.' replaced with "_" to make c++ happy



/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {}

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */
void autonomous() {

    if (SelectedAlliance == BLUE) {
        controller.print(0, 0, "BLUE");
    }
    else if (SelectedAlliance == RED){
        controller.print(0, 0, "RED");
    }
    get_selected_auton();
}

void opcontrol() {

    bool scorepiston = false;
	
	 while (true) {
        lv_timer_handler();
        // get joystick positions
        int leftY = controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
        int rightX = controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X);
        // move the chassis with curvature drive
        chassis.arcade(leftY, rightX);

        if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_A)){
            score();
        }
        
        if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_B)){
            if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_B)) {
                scorepiston.set_value(!scorepiston.get_value());
}
        }
        if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_Y)){
            scoremech.move(-100);
        }
    }
}
        if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_L1)){
            scoremech.move(100);
            
        }



