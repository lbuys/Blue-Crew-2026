#include "main.h"

#ifndef _AUTO_H_
#define _AUTO_H_

//Shared Declarations
void get_selected_auton();
void toggle();
const char* find_selected();
enum class IntakeDirection {
    In,
    Out,
    Score,
    Out_Slow,
    Score_Slow,
    Stop,
    Hold
};
extern IntakeDirection direction;
enum Alliance {
    BLUE,
    RED
};
extern Alliance SelectedAlliance;
void lvgl_initialize();
void intake(const std::string& direction);
void loader();
void loader_right();
void low_middle();
void high_middle();
void low_middle_new();
void skills();
void skills_2();
void wp();
void color_sort();
extern bool color_sorting;
void pid_tuner_start(double setpointX, double setpointY);
void pid_tuner_update_setpoint(double setpointX, double setpointY);
void pid_tuner_start_angle(double setpointHeadingDeg);
void pid_tuner_update_angle_setpoint(double setpointHeadingDeg);
void pid_tuner_stop();
void pid_tuner_show_menu();
void pid_tuner_run_selected_routine();
void show_auton_selector_screen();

#endif  // _AUTO_H_
