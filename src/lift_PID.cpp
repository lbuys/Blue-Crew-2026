#include "lift_PID.h"

double liftpositions[8] = {
    0,    // store
    100,  // Position 1
    200,  // Position 2
    300,  // Position 3
    400,  // Position 4
    500,  // Position 5
    600,  // Position 6
    700   // Position 7

}

double Kp = 0.5;
double Kd = 0;
double dt = 0.02;
int liftStage = 0;
int previous_height = 0;
liftenc.reset_position();
double target_height = liftpositions[liftStage];


void lift_PID() {
    while true:
        
        // flip scoring mech out if lift is above store position
        if (liftStage > 0){
            score.set_value(true);
        }
        
        current_height = liftenc.get_position();
         //Calculate deltas to the current target point
        double delta_height = target_height - current_height;

        double derivative =(current_height - previous_height) / dt;

        double speed = Kp * delta_height + Kd * derivative;

        // Set the lift motor speed
        lift_11W.move_velocity(speed);
        lift_half.move_velocity(speed);

        double previous_height = current_height;
        delay(20);  // Delay for 20 milliseconds 

}



void lift_movement_up() {

    bool lastPressed = false;


    uint32_t lastPressTime = 0;
    const uint32_t timeout = 1000;
    
    while (true) {
        
        bool pressed =
            master.get_digital(pros::E_CONTROLLER_DIGITAL_R1);

        if (pressed && !lastPressed) {

            liftStage++;

            if (liftStage > 8)
                liftStage = 8;

            target_height = liftPositions[liftStage];
        }

        lastPressed = pressed;

        pros::delay(10);
    }
}

void lift_movement_down() {

    bool lastPressed = false;


    uint32_t lastPressTime = 0;
    const uint32_t timeout = 1000;
    
    while (true) {
        
        bool pressed =
            master.get_digital(pros::E_CONTROLLER_DIGITAL_R2);

        if (pressed && !lastPressed) {

            liftStage--;

            if (liftStage < 0)
                liftStage = 0;

            target_height = liftPositions[liftStage];
        }

        lastPressed = pressed;

        pros::delay(10);
    }
}
void scoring_roller() {
    while (true) {
        if (direction == score_direction::score){
            scoring_mech.move_velocity(100);
        }
        else if (direction == score_direction::hold){
            scoring_mech.move_velocity(-100);
        }
        else if (direction == score_direction::off){
            scoring_mech.move_velocity(0);
        }
        pros::delay(50);
    }
}
void score()
{ 
    scorepiston.set_value(false);
    score_direction::score;
    pros::delay(500);
    score_direction::hold;
    target_height = current_height + 50;
    pros::delay(50);
    score.set_value(true);
    score_direction::off;

    }
