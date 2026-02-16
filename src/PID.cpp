#include "auto.h"
#include "config.h"
#include "liblvgl/widgets/chart/lv_chart.h"
#include "pros/rtos.hpp"

#include <algorithm>
#include <cstdint>
#include <cmath>

namespace {
// Internal mode for graph units and setpoint interpretation.
enum class TunerMode {
    LinearDistance,
    AngularHeading
};

constexpr lv_color_t kBg = LV_COLOR_MAKE(0xFF, 0xFF, 0xFF);
constexpr lv_color_t kCardBg = LV_COLOR_MAKE(0xF4, 0xF7, 0xFB);
constexpr lv_color_t kBorder = LV_COLOR_MAKE(0x00, 0x32, 0x63);
constexpr lv_color_t kGrid = LV_COLOR_MAKE(0xD9, 0xE2, 0xEC);
constexpr lv_color_t kSetpoint = LV_COLOR_MAKE(0x00, 0x5B, 0xB5);
constexpr lv_color_t kCurrent = LV_COLOR_MAKE(0x0B, 0x84, 0x6E);
constexpr lv_color_t kText = LV_COLOR_MAKE(0x18, 0x2B, 0x3A);
constexpr lv_color_t kSubtle = LV_COLOR_MAKE(0x5A, 0x6D, 0x7F);
constexpr double kDegToRad = 0.017453292519943295;  // pi / 180
constexpr std::uint32_t kLockWaitMs = 0xffffffffu;

lv_obj_t* prevScreen = nullptr;
lv_obj_t* tuneScreen = nullptr;
lv_obj_t* titleLabel = nullptr;
lv_obj_t* chart = nullptr;
lv_obj_t* setpointLabel = nullptr;
lv_obj_t* currentLabel = nullptr;
lv_obj_t* currentValueLabel = nullptr;
lv_obj_t* errorLabel = nullptr;
lv_obj_t* timeLabel = nullptr;
lv_obj_t* graphBackBtn = nullptr;
lv_chart_series_t* setpointSeries = nullptr;
lv_chart_series_t* currentSeries = nullptr;
lv_timer_t* updateTimer = nullptr;
lv_obj_t* tuningMenuScreen = nullptr;
lv_obj_t* tuningStatusLabel = nullptr;
bool tuningRoutineRunning = false;
bool tuningRoutineSelected = false;
pros::Mutex uiMutex;

enum class TuneRoutine {
    Lateral1,
    Lateral2,
    Angular1,
    Angular2
};

TuneRoutine selectedRoutine = TuneRoutine::Lateral1;

// Runtime graph state (setpoint and "current" are tracked in one scalar axis).
double targetX = 0.0;
double targetY = 0.0;
double startX = 0.0;
double startY = 0.0;
double startHeadingDeg = 0.0;
double targetHeadingDeg = 0.0;
double setpointPos = 0.0;
std::uint32_t runStartMs = 0;
bool pidUiActive = false;
TunerMode tunerMode = TunerMode::LinearDistance;

double normalize_angle_deg(double angleDeg) {
    while (angleDeg > 180.0) angleDeg -= 360.0;
    while (angleDeg < -180.0) angleDeg += 360.0;
    return angleDeg;
}

// Projects robot translation onto the starting heading axis so
// forward/back motion appears as signed 1D position over time.
double linear_position_from_start() {
    const lemlib::Pose pose = chassis.getPose();
    const double startHeadingRad = startHeadingDeg * kDegToRad;
    const double dx = pose.x - startX;
    const double dy = pose.y - startY;
    return dx * std::cos(startHeadingRad) + dy * std::sin(startHeadingRad);
}

// Converts a field target (x,y) into the same 1D axis used by the graph.
double linear_setpoint_from_start(double targetXIn, double targetYIn) {
    const double startHeadingRad = startHeadingDeg * kDegToRad;
    const double dx = targetXIn - startX;
    const double dy = targetYIn - startY;
    return dx * std::cos(startHeadingRad) + dy * std::sin(startHeadingRad);
}

double heading_delta_from_start() {
    const lemlib::Pose pose = chassis.getPose();
    return normalize_angle_deg(pose.theta - startHeadingDeg);
}

// Visual styling for the tuning graph panel.
void style_chart(lv_obj_t* c) {
    lv_obj_set_size(c, 460, 150);
    lv_obj_set_style_radius(c, 10, LV_PART_MAIN);
    lv_obj_set_style_border_color(c, kBorder, LV_PART_MAIN);
    lv_obj_set_style_border_width(c, 1, LV_PART_MAIN);
    lv_obj_set_style_bg_color(c, kCardBg, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(c, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_line_color(c, kGrid, LV_PART_MAIN);
    lv_obj_set_style_line_opa(c, LV_OPA_90, LV_PART_MAIN);
    lv_obj_set_style_line_width(c, 1, LV_PART_MAIN);
    lv_obj_set_style_line_width(c, 2, LV_PART_ITEMS);
    lv_chart_set_type(c, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(c, 500);
    lv_chart_set_update_mode(c, LV_CHART_UPDATE_MODE_SHIFT);
    lv_chart_set_div_line_count(c, 10, 8);
    lv_chart_set_range(c, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
}

// Starts a fresh linear run: reset timeline, zero chart history, and set first target.
void begin_run(double setpointX, double setpointY) {
    tunerMode = TunerMode::LinearDistance;
    targetX = setpointX;
    targetY = setpointY;
    const lemlib::Pose pose = chassis.getPose();
    startX = pose.x;
    startY = pose.y;
    startHeadingDeg = pose.theta;
    setpointPos = linear_setpoint_from_start(targetX, targetY);
    runStartMs = pros::millis();

    if (chart != nullptr && setpointSeries != nullptr && currentSeries != nullptr) {
        lv_chart_set_all_value(chart, setpointSeries, 0);
        lv_chart_set_all_value(chart, currentSeries, 0);
        lv_chart_refresh(chart);
    }
}

// Changes the linear setpoint without clearing history so multi-step tests stay on one graph.
void update_linear_setpoint(double setpointX, double setpointY) {
    targetX = setpointX;
    targetY = setpointY;
    setpointPos = linear_setpoint_from_start(targetX, targetY);
}

// Starts a fresh angular run with heading-relative setpoint.
void begin_angle_run(double setpointHeadingDeg) {
    tunerMode = TunerMode::AngularHeading;
    const lemlib::Pose pose = chassis.getPose();
    startHeadingDeg = pose.theta;
    targetHeadingDeg = setpointHeadingDeg;
    setpointPos = normalize_angle_deg(targetHeadingDeg - startHeadingDeg);
    runStartMs = pros::millis();

    if (chart != nullptr && setpointSeries != nullptr && currentSeries != nullptr) {
        lv_chart_set_all_value(chart, setpointSeries, 0);
        lv_chart_set_all_value(chart, currentSeries, 0);
        lv_chart_refresh(chart);
    }
}

// Changes angular target without resetting accumulated graph data.
void update_angle_setpoint(double setpointHeadingDeg) {
    targetHeadingDeg = setpointHeadingDeg;
    setpointPos = normalize_angle_deg(targetHeadingDeg - startHeadingDeg);
}

// Timer callback: append one sample (setpoint/current), auto-scale y-axis, and update stats text.
void update_graph(lv_timer_t*) {
    if (!pidUiActive) return;

    const double currentPos = (tunerMode == TunerMode::LinearDistance)
        ? linear_position_from_start()
        : heading_delta_from_start();

    const double minV = std::min(setpointPos, currentPos);
    const double maxV = std::max(setpointPos, currentPos);
    const double span = std::max(10.0, maxV - minV);
    const double pad = std::max(2.0, span * 0.20);
    const int yMin = static_cast<int>(std::floor(minV - pad));
    const int yMax = static_cast<int>(std::ceil(maxV + pad));
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, yMin, std::max(yMin + 1, yMax));

    lv_chart_set_next_value(chart, setpointSeries, static_cast<lv_coord_t>(std::lround(setpointPos)));
    lv_chart_set_next_value(chart, currentSeries, static_cast<lv_coord_t>(std::lround(currentPos)));
    lv_chart_refresh(chart);

    const double elapsedSec = static_cast<double>(pros::millis() - runStartMs) / 1000.0;
    const double error = setpointPos - currentPos;
    if (tunerMode == TunerMode::LinearDistance) {
        lv_label_set_text_fmt(currentValueLabel, "Current value: %.2f in", currentPos);
        lv_label_set_text_fmt(errorLabel, "Error: %.2f in", error);
    } else {
        lv_label_set_text_fmt(currentValueLabel, "Current value: %.2f deg", currentPos);
        lv_label_set_text_fmt(errorLabel, "Error: %.2f deg", error);
    }
    lv_label_set_text_fmt(timeLabel, "Total time: %.1fs", elapsedSec);
}

// Graph screen navigation: after a run, go back to tuning menu (or selector fallback).
void graph_back_cb(lv_event_t* e) {
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (tuningRoutineRunning) return;
    if (tuningMenuScreen != nullptr) {
        lv_screen_load(tuningMenuScreen);
    } else {
        show_auton_selector_screen();
    }
}

// Built-in linear tuning routine #1:
// +48 in, then -24 in, then -24 in (relative sequence shown as setpoint steps).
void run_lateral_routine_1() {
    chassis.setPose(0, 0, 0);
    pid_tuner_start(48, 0);
    chassis.moveToPoint(48, 0, 4000, {}, false);
    pid_tuner_update_setpoint(24, 0);
    chassis.moveToPoint(24, 0, 3000, {}, false);
    pid_tuner_update_setpoint(0, 0);
    chassis.moveToPoint(0, 0, 3000, {}, false);
    pid_tuner_stop();
}

// Built-in linear tuning routine #2: single +48 in move.
void run_lateral_routine_2() {
    chassis.setPose(0, 0, 0);
    pid_tuner_start(48, 0);
    chassis.moveToPoint(48, 0, 4000, {}, false);
    pid_tuner_stop();
}

// Built-in angular tuning routine #1:
// +90 deg, then +90 deg, then -180 deg back to start heading.
void run_angular_routine_1() {
    chassis.setPose(0, 0, 0);
    pid_tuner_start_angle(90);
    chassis.turnToHeading(90, 2500, {}, false);
    pid_tuner_update_angle_setpoint(180);
    chassis.turnToHeading(180, 2500, {}, false);
    pid_tuner_update_angle_setpoint(0);
    chassis.turnToHeading(0, 3000, {}, false);
    pid_tuner_stop();
}

// Built-in angular tuning routine #2: single +90 deg turn.
void run_angular_routine_2() {
    chassis.setPose(0, 0, 0);
    pid_tuner_start_angle(90);
    chassis.turnToHeading(90, 2500, {}, false);
    pid_tuner_stop();
}

// Human-readable label for menu/status text.
const char* routine_name(TuneRoutine routine) {
    switch (routine) {
        case TuneRoutine::Lateral1: return "Lateral Tune 1";
        case TuneRoutine::Lateral2: return "Lateral Tune 2";
        case TuneRoutine::Angular1: return "Angular Tune 1";
        case TuneRoutine::Angular2: return "Angular Tune 2";
    }
    return "Unknown";
}

// Pre-auton selection only; execution happens later in autonomous.
void select_tuning_routine(TuneRoutine routine) {
    selectedRoutine = routine;
    tuningRoutineSelected = true;
    if (tuningStatusLabel != nullptr) {
        lv_label_set_text_fmt(tuningStatusLabel, "Selected: %s", routine_name(routine));
    }
}

// Tuning menu callbacks (select routine / navigate).
void back_btn_cb(lv_event_t* e) {
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (tuningRoutineRunning) return;
    show_auton_selector_screen();
}

void lateral_1_cb(lv_event_t* e) {
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    select_tuning_routine(TuneRoutine::Lateral1);
}

void lateral_2_cb(lv_event_t* e) {
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    select_tuning_routine(TuneRoutine::Lateral2);
}

void angular_1_cb(lv_event_t* e) {
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    select_tuning_routine(TuneRoutine::Angular1);
}

void angular_2_cb(lv_event_t* e) {
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    select_tuning_routine(TuneRoutine::Angular2);
}

lv_obj_t* add_menu_btn(lv_obj_t* parent, const char* txt, lv_event_cb_t cb, lv_coord_t x, lv_coord_t y) {
    lv_obj_t* btn = lv_button_create(parent);
    lv_obj_set_size(btn, 220, 44);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, x, y);
    lv_obj_set_style_radius(btn, 10, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, kBorder, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text(label, txt);
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    lv_obj_center(label);
    return btn;
}

}  // namespace

// Public API: open/reuse graph in linear mode.
void pid_tuner_start(double setpointX, double setpointY) {
    uiMutex.take(kLockWaitMs);
    if (pidUiActive) {
        if (updateTimer == nullptr) {
            updateTimer = lv_timer_create(update_graph, 50, nullptr);
        }
        begin_run(setpointX, setpointY);
        uiMutex.give();
        return;
    }

    prevScreen = lv_screen_active();
    tuneScreen = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(tuneScreen, kBg, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(tuneScreen, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_remove_flag(tuneScreen, LV_OBJ_FLAG_SCROLLABLE);

    titleLabel = lv_label_create(tuneScreen);
    lv_obj_set_style_text_color(titleLabel, kBorder, LV_PART_MAIN);
    lv_obj_align(titleLabel, LV_ALIGN_TOP_LEFT, 10, 5);
    lv_label_set_text(titleLabel, "PID Tuning");

    setpointLabel = lv_label_create(tuneScreen);
    lv_obj_set_style_text_color(setpointLabel, kSetpoint, LV_PART_MAIN);
    lv_obj_align(setpointLabel, LV_ALIGN_TOP_LEFT, 10, 24);
    lv_label_set_text(setpointLabel, "Setpoint");

    currentLabel = lv_label_create(tuneScreen);
    lv_obj_set_style_text_color(currentLabel, kCurrent, LV_PART_MAIN);
    lv_obj_align(currentLabel, LV_ALIGN_TOP_LEFT, 10, 40);
    lv_label_set_text(currentLabel, "Current value");

    chart = lv_chart_create(tuneScreen);
    style_chart(chart);
    lv_obj_align(chart, LV_ALIGN_TOP_LEFT, 10, 60);
    setpointSeries = lv_chart_add_series(chart, kSetpoint, LV_CHART_AXIS_PRIMARY_Y);
    currentSeries = lv_chart_add_series(chart, kCurrent, LV_CHART_AXIS_PRIMARY_Y);

    currentValueLabel = lv_label_create(tuneScreen);
    lv_obj_set_style_text_color(currentValueLabel, kText, LV_PART_MAIN);
    lv_obj_align(currentValueLabel, LV_ALIGN_TOP_LEFT, 10, 214);
    lv_label_set_text(currentValueLabel, "Current value: 0.00");

    errorLabel = lv_label_create(tuneScreen);
    lv_obj_set_style_text_color(errorLabel, kText, LV_PART_MAIN);
    lv_obj_align(errorLabel, LV_ALIGN_TOP_LEFT, 10, 232);
    lv_label_set_text(errorLabel, "Error: 0.00");

    timeLabel = lv_label_create(tuneScreen);
    lv_obj_set_style_text_color(timeLabel, kSubtle, LV_PART_MAIN);
    lv_obj_align(timeLabel, LV_ALIGN_TOP_RIGHT, -120, 232);
    lv_label_set_text(timeLabel, "Total time: 0.0s");

    graphBackBtn = lv_button_create(tuneScreen);
    lv_obj_set_size(graphBackBtn, 100, 32);
    lv_obj_align(graphBackBtn, LV_ALIGN_TOP_RIGHT, -10, 228);
    lv_obj_set_style_radius(graphBackBtn, 10, LV_PART_MAIN);
    lv_obj_set_style_bg_color(graphBackBtn, lv_color_hex(0x808080), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(graphBackBtn, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_add_event_cb(graphBackBtn, graph_back_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_t* graphBackLabel = lv_label_create(graphBackBtn);
    lv_label_set_text(graphBackLabel, "Back");
    lv_obj_set_style_text_color(graphBackLabel, lv_color_white(), LV_PART_MAIN);
    lv_obj_center(graphBackLabel);

    lv_screen_load(tuneScreen);
    pidUiActive = true;
    begin_run(setpointX, setpointY);
    updateTimer = lv_timer_create(update_graph, 50, nullptr);
    uiMutex.give();
}

// Public API: update linear setpoint while keeping existing run trace.
void pid_tuner_update_setpoint(double setpointX, double setpointY) {
    uiMutex.take(kLockWaitMs);
    update_linear_setpoint(setpointX, setpointY);
    uiMutex.give();
}

// Public API: open/reuse graph in angular mode.
void pid_tuner_start_angle(double setpointHeadingDeg) {
    uiMutex.take(kLockWaitMs);
    if (pidUiActive) {
        if (updateTimer == nullptr) {
            updateTimer = lv_timer_create(update_graph, 50, nullptr);
        }
        begin_angle_run(setpointHeadingDeg);
        uiMutex.give();
        return;
    }
    uiMutex.give();
    pid_tuner_start(0.0, 0.0);
    uiMutex.take(kLockWaitMs);
    begin_angle_run(setpointHeadingDeg);
    uiMutex.give();
}

// Public API: update angular setpoint while keeping existing run trace.
void pid_tuner_update_angle_setpoint(double setpointHeadingDeg) {
    uiMutex.take(kLockWaitMs);
    update_angle_setpoint(setpointHeadingDeg);
    uiMutex.give();
}

// Public API: stop sampling but keep graph visible for post-run analysis.
void pid_tuner_stop() {
    uiMutex.take(kLockWaitMs);
    if (!pidUiActive) {
        uiMutex.give();
        return;
    }

    if (updateTimer != nullptr) {
        lv_timer_delete(updateTimer);
        updateTimer = nullptr;
    }

    // Freeze graph in place for post-run analysis.
    uiMutex.give();
}

// Public API: show/create pre-auton tuning selection menu.
void pid_tuner_show_menu() {
    if (tuningMenuScreen == nullptr) {
        tuningMenuScreen = lv_obj_create(nullptr);
        lv_obj_set_style_bg_color(tuningMenuScreen, kBg, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(tuningMenuScreen, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_remove_flag(tuningMenuScreen, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t* title = lv_label_create(tuningMenuScreen);
        lv_label_set_text(title, "PID Tuning Menu");
        lv_obj_set_style_text_color(title, kBorder, LV_PART_MAIN);
        lv_obj_align(title, LV_ALIGN_TOP_LEFT, 10, 8);

        lv_obj_t* sub = lv_label_create(tuningMenuScreen);
        lv_label_set_text(sub, "Select a tuning auton");
        lv_obj_set_style_text_color(sub, kSubtle, LV_PART_MAIN);
        lv_obj_align(sub, LV_ALIGN_TOP_LEFT, 10, 28);

        add_menu_btn(tuningMenuScreen, "Lateral Tune 1", lateral_1_cb, 10, 58);
        add_menu_btn(tuningMenuScreen, "Lateral Tune 2", lateral_2_cb, 10, 108);
        add_menu_btn(tuningMenuScreen, "Angular Tune 1", angular_1_cb, 250, 58);
        add_menu_btn(tuningMenuScreen, "Angular Tune 2", angular_2_cb, 250, 108);

        lv_obj_t* backBtn = lv_button_create(tuningMenuScreen);
        lv_obj_set_size(backBtn, 100, 40);
        lv_obj_align(backBtn, LV_ALIGN_BOTTOM_LEFT, 10, -10);
        lv_obj_set_style_radius(backBtn, 10, LV_PART_MAIN);
        lv_obj_set_style_bg_color(backBtn, lv_color_hex(0x808080), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(backBtn, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_add_event_cb(backBtn, back_btn_cb, LV_EVENT_CLICKED, nullptr);
        lv_obj_t* backLabel = lv_label_create(backBtn);
        lv_label_set_text(backLabel, "Back");
        lv_obj_set_style_text_color(backLabel, lv_color_white(), LV_PART_MAIN);
        lv_obj_center(backLabel);

        tuningStatusLabel = lv_label_create(tuningMenuScreen);
        lv_obj_set_style_text_color(tuningStatusLabel, kText, LV_PART_MAIN);
        lv_obj_align(tuningStatusLabel, LV_ALIGN_BOTTOM_RIGHT, -10, -20);
        lv_label_set_text(tuningStatusLabel, "Selected: none");
    }

    if (tuningStatusLabel != nullptr && !tuningRoutineRunning) {
        if (tuningRoutineSelected) {
            lv_label_set_text_fmt(tuningStatusLabel, "Selected: %s", routine_name(selectedRoutine));
        } else {
            lv_label_set_text(tuningStatusLabel, "Selected: none");
        }
    }
    lv_screen_load(tuningMenuScreen);
}

// Public API: called from autonomous when "tuning" auton is selected.
// Runs the preselected routine and updates menu status.
void pid_tuner_run_selected_routine() {
    if (!tuningRoutineSelected) {
        controller.print(0, 0, "Select tuning routine in pre-auton");
        return;
    }

    tuningRoutineRunning = true;
    if (tuningStatusLabel != nullptr) {
        lv_label_set_text_fmt(tuningStatusLabel, "Running: %s", routine_name(selectedRoutine));
    }

    switch (selectedRoutine) {
        case TuneRoutine::Lateral1: run_lateral_routine_1(); break;
        case TuneRoutine::Lateral2: run_lateral_routine_2(); break;
        case TuneRoutine::Angular1: run_angular_routine_1(); break;
        case TuneRoutine::Angular2: run_angular_routine_2(); break;
    }

    tuningRoutineRunning = false;
    if (tuningStatusLabel != nullptr) {
        lv_label_set_text_fmt(tuningStatusLabel, "Done: %s", routine_name(selectedRoutine));
    }
}
