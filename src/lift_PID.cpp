#include "lift_PID.h"

#include "auto.h"
#include "config.h"

#include <algorithm>

namespace {
constexpr double liftPositions[] = {0, 50, 100, 150, 200, 250, 300, 350};
constexpr double kP = 20;
constexpr double kMaxLiftVelocity = 200.0;
double liftBaseHeight = 0;
bool liftSensorReady = false;
pros::Mutex liftStateMutex;

void display_lift_stage() {
    controller.print(1, 0, "Height: %d", liftStage);
}
}

int liftStage = 0;
double current_height = 0;
double target_height = 0;
IntakeDirection direction = IntakeDirection::Stop;

void lift_controller_initialize() {
    liftSensorReady = false;
    liftStage = 0;
    current_height = 0;
    liftBaseHeight = 0;
    target_height = 0;
    display_lift_stage();
}

void lift_PID() {
    const std::int32_t sensorPosition = liftenc.get_position();
    if (sensorPosition == PROS_ERR) {
        // Never feed a PROS error sentinel into the PID calculation.
        lift_11W.move_velocity(0);
        lift_half.move_velocity(0);
        liftSensorReady = false;
        return;
    }

    current_height = sensorPosition / 100.0; // PROS reports rotation in centidegrees.
    if (!liftSensorReady) {
        // Establish a zero reference only after the sensor returns a valid value.
        liftBaseHeight = current_height;
        target_height = liftBaseHeight;
        liftStage = 0;
        liftSensorReady = true;
    }

    liftStateMutex.take();
    const double error = target_height - current_height;
    const int stage = liftStage;
    liftStateMutex.give();
    const double velocity = std::clamp(kP * error, -kMaxLiftVelocity, kMaxLiftVelocity);

    lift_11W.move_velocity(velocity);
    lift_half.move_velocity(velocity);
    // Keep the scoring mechanism safely stowed at the bottom stage.
    if (stage == 0) scorepiston.set_value(false);
}

void lift_movement_up() {
    static bool wasPressed = false;
    const bool pressed = controller.get_digital(pros::E_CONTROLLER_DIGITAL_R1);
    if (pressed && !wasPressed) {
        liftStateMutex.take();
        liftStage = std::min(liftStage + 1, 7);
        target_height = liftBaseHeight + liftPositions[liftStage];
        liftStateMutex.give();
        display_lift_stage();
    }
    wasPressed = pressed;
}

void lift_movement_down() {
    static bool wasPressed = false;
    const bool pressed = controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2);
    if (pressed && !wasPressed) {
        liftStateMutex.take();
        liftStage = std::max(liftStage - 1, 0);
        target_height = liftBaseHeight + liftPositions[liftStage];
        liftStateMutex.give();
        display_lift_stage();
    }
    wasPressed = pressed;
}

void lift_control_task() {
    while (true) {
        lift_PID(); // Validate/read the sensor before accepting a new target.
        lift_movement_up();
        lift_movement_down();
        pros::delay(20);
    }
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
    liftStateMutex.take();
    liftStage = std::min(liftStage + 1, 7);
    target_height = liftBaseHeight + liftPositions[liftStage];
    liftStateMutex.give();
    display_lift_stage();
    pros::delay(50);

    scorepiston.set_value(true);
    direction = IntakeDirection::Stop;
}
