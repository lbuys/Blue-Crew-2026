#include "main.h"

#ifndef _AUTO_H_
#define _AUTO_H_

//Shared Declarations
void get_selected_auton();
const char* find_selected();
enum class IntakeDirection {
    In,
    Out,
    Score,
    Out_Slow,
    Score_Slow,
    Stop
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

#endif  // _AUTO_H_