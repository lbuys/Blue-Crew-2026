#include "lift_PID.h"

#include "auto.h"
#include "config.h"

#include <algorithm>
#include <cmath>

namespace {
// Preset positions are absolute liftenc readings in degrees, not offsets.
// Replace these example values with readings taken at each desired height.
constexpr double liftPositionsDegrees[] = {0, 100, 200, 300, 400, 500, 600, 700};
constexpr double kP = 0.8;
constexpr double kI = 0.02;
constexpr double kD = 0.05;
constexpr double kMaxLiftVelocity = 200.0;
constexpr double kLiftTolerance = 2.0;
constexpr double kLoopSeconds = 0.02;
constexpr double kIntegralLimit = 500.0;
bool liftSensorReady = false;
int manualLiftVelocity = 0;
pros::Mutex liftStateMutex;

void display_lift_stage() {
    controller.print(1, 0, "Stage: %d", liftStage);
    controller.print(2, 0, "Sensor: %7.1f", current_height);
}
}

int liftStage = 0;
double current_height = 0;
double target_height = 0;
IntakeDirection direction = IntakeDirection::Stop;

void lift_controller_initialize() {
    lift_11W.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
    lift_half.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
    liftSensorReady = false;
    liftStage = 0;
    current_height = 0;
    target_height = 0;
    manualLiftVelocity = 0;
    display_lift_stage();
}

void lift_PID() {
    static double integral = 0;
    static double previousError = 0;
    static double previousTarget = 0;
    static bool hasPreviousError = false;
    const std::int32_t sensorPosition = liftenc.get_position();
    if (sensorPosition == PROS_ERR) {
        // Never feed a PROS error sentinel into the PID calculation.
        lift_11W.move_velocity(0);
        lift_half.move_velocity(0);
        integral = 0;
        hasPreviousError = false;
        return;
    }

    current_height = sensorPosition / 100.0; // PROS reports centidegrees; targets use degrees.
    if (!liftSensorReady) {
        // Hold the startup position without changing the sensor's coordinate system.
        target_height = current_height;
        liftStage = 0;
        liftSensorReady = true;
    }

    liftStateMutex.take();
    const double error = target_height - current_height;
    const int stage = liftStage;
    const double target = target_height;
    liftStateMutex.give();

    if (!hasPreviousError || target != previousTarget) {
        integral = 0;
        previousError = error;
        previousTarget = target;
        hasPreviousError = true;
    }

    double velocity = 0;
    if (std::abs(error) > kLiftTolerance) {
        const double derivative = (error - previousError) / kLoopSeconds;
        const double candidateIntegral = std::clamp(integral + error * kLoopSeconds,
                                                    -kIntegralLimit, kIntegralLimit);
        const double candidateOutput = kP * error + kI * candidateIntegral + kD * derivative;
        // Stop the integral from winding up while the motor command is capped.
        if (std::abs(candidateOutput) <= kMaxLiftVelocity ||
            (candidateOutput > kMaxLiftVelocity && error < 0) ||
            (candidateOutput < -kMaxLiftVelocity && error > 0)) {
            integral = candidateIntegral;
        }
        velocity = std::clamp(kP * error + kI * integral + kD * derivative,
                              -kMaxLiftVelocity, kMaxLiftVelocity);
    } else {
        integral = 0;
    }
    previousError = error;

    lift_11W.move_velocity(velocity);
    lift_half.move_velocity(velocity);
    display_lift_stage();
    // Keep the scoring mechanism safely stowed at the bottom stage.
    if (stage == 0) scorepiston.set_value(false);
}

void lift_movement_up() {
    static bool wasPressed = false;
    const bool pressed = controller.get_digital(pros::E_CONTROLLER_DIGITAL_R1) &&
                         !controller.get_digital(pros::E_CONTROLLER_DIGITAL_L2);
    if (pressed && !wasPressed) {
        liftStateMutex.take();
        liftStage = std::min(liftStage + 1, 7);
        target_height = liftPositionsDegrees[liftStage];
        liftStateMutex.give();
        display_lift_stage();
    }
    wasPressed = pressed;
}

void lift_movement_down() {
    static bool wasPressed = false;
    const bool pressed = controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2) &&
                         !controller.get_digital(pros::E_CONTROLLER_DIGITAL_L2);
    if (pressed && !wasPressed) {
        liftStateMutex.take();
        liftStage = std::max(liftStage - 1, 0);
        target_height = liftPositionsDegrees[liftStage];
        liftStateMutex.give();
        display_lift_stage();
    }
    wasPressed = pressed;
}

void lift_control_task() {
    bool manualWasActive = false;
    while (true) {
        liftStateMutex.take();
        const int requestedManualVelocity = manualLiftVelocity;
        liftStateMutex.give();
        const bool manualActive = requestedManualVelocity != 0;
        const std::int32_t sensorPosition = liftenc.get_position();
        const bool sensorValid = sensorPosition != PROS_ERR;

        if (manualActive) {
            // Driver backup requests pass through this task so motor output has one owner.
            lift_11W.move_velocity(requestedManualVelocity);
            lift_half.move_velocity(requestedManualVelocity);
        } else {
            if (manualWasActive && sensorValid) {
                // Resume PID by holding the position where manual control ended.
                liftStateMutex.take();
                target_height = sensorPosition / 100.0;
                liftStateMutex.give();
            }

            lift_PID(); // PID owns motor output during normal operation.
        }
        // R1/R2 select preset stages; hold L2 with them for manual backup.
        lift_movement_up();
        lift_movement_down();
        manualWasActive = manualActive;
        pros::delay(20);
    }
}

void lift_set_manual_velocity(int velocity) {
    liftStateMutex.take();
    manualLiftVelocity = std::clamp(velocity, -100, 100);
    liftStateMutex.give();
}

void lift_set_target_degrees(double targetDegrees) {
    if (!std::isfinite(targetDegrees)) return;

    int nearestStage = 0;
    double nearestDistance = std::abs(liftPositionsDegrees[0] - targetDegrees);
    for (int i = 1; i < 8; ++i) {
        const double distance = std::abs(liftPositionsDegrees[i] - targetDegrees);
        if (distance < nearestDistance) {
            nearestDistance = distance;
            nearestStage = i;
        }
    }

    liftStateMutex.take();
    target_height = targetDegrees;
    liftStage = nearestStage;
    liftStateMutex.give();
    display_lift_stage();
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
    target_height = liftPositionsDegrees[liftStage];
    liftStateMutex.give();
    display_lift_stage();
    pros::delay(50);

    scorepiston.set_value(true);
    direction = IntakeDirection::Stop;
}
