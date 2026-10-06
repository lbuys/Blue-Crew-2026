#include "lift_PID.h"

#include "auto.h"
#include "config.h"

#include <algorithm>

namespace {
constexpr double liftPositions[] = {0, 100, 200, 300, 400, 500, 600, 700};
constexpr double kP = 0.5;
constexpr double kMaxLiftVelocity = 200.0;
}

int liftStage = 0;
double current_height = 0;
double target_height = 0;
IntakeDirection direction = IntakeDirection::Stop;

void lift_PID() {
    current_height = liftenc.get_position();
    const double error = target_height - current_height;
    const double velocity = std::clamp(kP * error, -kMaxLiftVelocity, kMaxLiftVelocity);

    lift_11W.move_velocity(velocity);
    lift_half.move_velocity(velocity);
    // Keep the scoring mechanism safely stowed at the bottom stage.
    if (liftStage == 0) scorepiston.set_value(false);
}

void lift_movement_up() {
    static bool wasPressed = false;
    const bool pressed = controller.get_digital(pros::E_CONTROLLER_DIGITAL_R1);
    if (pressed && !wasPressed) {
        liftStage = std::min(liftStage + 1, 7);
        target_height = liftPositions[liftStage];
    }
    wasPressed = pressed;
}

void lift_movement_down() {
    static bool wasPressed = false;
    const bool pressed = controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2);
    if (pressed && !wasPressed) {
        liftStage = std::max(liftStage - 1, 0);
        target_height = liftPositions[liftStage];
    }
    wasPressed = pressed;
}

void scoring_roller() {
    switch (direction) {
        case IntakeDirection::Score:
            scoring_mech.move_velocity(100);
            break;
        case IntakeDirection::Score_Slow:
            scoring_mech.move_velocity(50);
            break;
        case IntakeDirection::In:
            scoring_mech.move_velocity(100);
            break;
        case IntakeDirection::Out:
            scoring_mech.move_velocity(-100);
            break;
        case IntakeDirection::Out_Slow:
            scoring_mech.move_velocity(-50);
            break;
        case IntakeDirection::Hold:
            scoring_mech.move_velocity(-20);
            break;
        case IntakeDirection::Stop:
            scoring_mech.move_velocity(0);
            break;
    }
}

void score() {
    scorepiston.set_value(false);
    direction = IntakeDirection::Score;
    pros::delay(500);

    direction = IntakeDirection::Hold;
    current_height = liftenc.get_position();
    target_height = current_height + 50;
    liftStage = std::min(liftStage + 1, 7);
    pros::delay(50);

    scorepiston.set_value(true);
    direction = IntakeDirection::Stop;
}
