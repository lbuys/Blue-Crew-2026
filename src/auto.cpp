#include "main.h"
#include "lemlib/api.hpp"
#include "lemlib/chassis/trackingWheel.hpp"
#include "liblvgl/core/lv_obj.h"
#include "liblvgl/misc/lv_area.h"
#include "liblvgl/widgets/image/lv_image.h"
#include "pros/adi.hpp"
#include "pros/motors.h"
#include "pros/motors.hpp"
#include "pros/optical.hpp"
#include "pros/rotation.hpp"
#include "auto.h"

lv_obj_t *leftlist;
lv_obj_t *rightlist;
lv_obj_t *skillslist;
lv_obj_t * btnLeft;
lv_obj_t * btnRight;
lv_obj_t * btnSkills;
lv_obj_t * list_group_t;
// lv_obj_t* img = lv_img_create(lv_scr_act());  // Creates image object
lv_obj_t* img = nullptr;

void list_btn_event_c(lv_event_t *e) {
    lv_obj_t *clicked_btn = lv_event_get_target(e);

    // Go through all lists and clear check from every button
    lv_obj_t *lists[] = { leftlist, rightlist, skillslist };
    for (int i = 0; i < 3; i++) {
        uint32_t child_cnt = lv_obj_get_child_cnt(lists[i]);
        for (uint32_t j = 0; j < child_cnt; j++) {
            lv_obj_t *btn = lv_obj_get_child(lists[i], j);
            lv_obj_clear_state(btn, LV_STATE_CHECKED);
        }
    }

    // Check only the clicked button
    lv_obj_add_state(clicked_btn, LV_STATE_CHECKED);
}


void create_btn(lv_obj_t *parent, const char *txt) { //Creates button with user data
    lv_obj_t *btn = lv_list_add_btn(parent, NULL, txt);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_style_radius(btn, 10, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x808080), LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, LV_STATE_CHECKED);
    char* txt_copy = strdup(txt);
    lv_obj_set_user_data(btn, txt_copy);
    lv_obj_add_event_cb(btn, list_btn_event_c, LV_EVENT_CLICKED, NULL);
}

static void LeftEventCb(lv_event_t *e) { //Left click back event
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    // lv_obj_clear_flag(leftlist, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(rightlist, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(skillslist, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(leftlist, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_state(btnRight, LV_STATE_CHECKED);
    lv_obj_clear_state(btnSkills, LV_STATE_CHECKED);
  }
}
static void RightEventCb(lv_event_t *e) { //Right click back event
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    // lv_obj_clear_flag(leftlist, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(leftlist, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(skillslist, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(rightlist, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_state(btnLeft, LV_STATE_CHECKED);
    lv_obj_clear_state(btnSkills, LV_STATE_CHECKED);
  }
}
static void SkillsEventCb(lv_event_t *e) { //Skills click back event
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    // lv_obj_clear_flag(leftlist, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(leftlist, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(rightlist, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(skillslist, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_state(btnLeft, LV_STATE_CHECKED);
    lv_obj_clear_state(btnRight, LV_STATE_CHECKED);
  }
}

static void UCLogoEventCb(lv_event_t * e) { //UC logo click back event
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        if (SelectedAlliance == BLUE){
          SelectedAlliance = RED;
          lv_obj_set_style_bg_color(btnLeft, lv_color_hex(0xee2a36), LV_PART_MAIN); //Red
          lv_obj_set_style_bg_color(btnRight, lv_color_hex(0xee2a36), LV_PART_MAIN);
          lv_obj_set_style_bg_color(btnSkills, lv_color_hex(0xee2a36), LV_PART_MAIN);
          LV_IMG_DECLARE(red_uc);
          lv_img_set_src(img, &red_uc); // Print RED UC logo
        }
        else if (SelectedAlliance == RED){
          SelectedAlliance = BLUE;
          lv_obj_set_style_bg_color(btnLeft, lv_color_hex(0x003263), LV_PART_MAIN); //Blue
          lv_obj_set_style_bg_color(btnRight, lv_color_hex(0x003263), LV_PART_MAIN);
          lv_obj_set_style_bg_color(btnSkills, lv_color_hex(0x003263), LV_PART_MAIN);
          LV_IMG_DECLARE(uc);
          lv_img_set_src(img, &uc);  // Print BLUE UC logo
        }
    }
}


void lvgl_initialize() {
    // Creates Main Screen
    lv_obj_t * main_screen = lv_obj_create(NULL);
    lv_scr_load(main_screen);

    // LV_IMG_DECLARE(uc);    //              at the top
    // lv_obj_t* img = lv_img_create(lv_scr_act());  // Creates image object            moved to top
    img = lv_img_create(main_screen);
    LV_IMG_DECLARE(uc);
    lv_img_set_src(img, &uc);                  // Make UC logo
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(img, LV_ALIGN_TOP_RIGHT, 22, 0);
    lv_img_set_zoom(img, 200);
    lv_obj_add_event_cb(img, UCLogoEventCb, LV_EVENT_CLICKED, NULL);

    lv_obj_set_style_bg_color(main_screen, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(main_screen, LV_OPA_COVER, LV_PART_MAIN);

    // Creates three separate buttons at the bottom for, Left / Right / Skills

    // Left button
    btnLeft = lv_btn_create(main_screen);
    lv_obj_set_width(btnLeft, lv_pct(30));
    lv_obj_align(btnLeft, LV_ALIGN_BOTTOM_LEFT,  10, -10);
    lv_obj_add_flag(btnLeft, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_style_bg_color(btnLeft, lv_color_hex(0x003263), LV_PART_MAIN);          // blue when not toggled
    lv_obj_set_style_bg_opa  (btnLeft, LV_OPA_COVER,    LV_PART_MAIN);
    lv_obj_set_style_bg_color(btnLeft, lv_color_hex(0x808080), LV_STATE_CHECKED);       // gray when toggled
    lv_obj_set_style_bg_opa  (btnLeft, LV_OPA_COVER,    LV_STATE_CHECKED);
    // Creates a label inside the Left button
    lv_obj_t * lblLeft = lv_label_create(btnLeft);
    lv_label_set_text(lblLeft, "Left");
    lv_obj_center(lblLeft);
    // Attach the callback to button
    lv_obj_add_event_cb(btnLeft, LeftEventCb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_state(btnLeft, LV_STATE_CHECKED);

    // Right button
    btnRight = lv_btn_create(main_screen);
    lv_obj_set_width(btnRight, lv_pct(30));
    lv_obj_align(btnRight, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_add_flag(btnRight, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_style_bg_color(btnRight, lv_color_hex(0x003263), LV_PART_MAIN);          // blue when not toggled
    lv_obj_set_style_bg_opa  (btnRight, LV_OPA_COVER,    LV_PART_MAIN);
    lv_obj_set_style_bg_color(btnRight, lv_color_hex(0x808080), LV_STATE_CHECKED);       // gray when toggled
    lv_obj_set_style_bg_opa  (btnRight, LV_OPA_COVER,    LV_STATE_CHECKED);
    lv_obj_t * lblRight = lv_label_create(btnRight);
    lv_label_set_text(lblRight, "Right");
    lv_obj_center(lblRight);
    lv_obj_add_event_cb(btnRight, RightEventCb, LV_EVENT_CLICKED, NULL);

    // Skills button
    btnSkills = lv_btn_create(main_screen);
    lv_obj_set_width(btnSkills, lv_pct(30));
    lv_obj_align(btnSkills, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
    lv_obj_add_flag(btnSkills, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_style_bg_color(btnSkills, lv_color_hex(0x003263), LV_PART_MAIN);          // blue when not toggled
    lv_obj_set_style_bg_opa  (btnSkills, LV_OPA_COVER,    LV_PART_MAIN);
    lv_obj_set_style_bg_color(btnSkills, lv_color_hex(0x808080), LV_STATE_CHECKED);       // gray when toggled
    lv_obj_set_style_bg_opa  (btnSkills, LV_OPA_COVER,    LV_STATE_CHECKED);
    lv_obj_t * lblSkills = lv_label_create(btnSkills);
    lv_label_set_text(lblSkills, "Skills");
    lv_obj_center(lblSkills);
    lv_obj_add_event_cb(btnSkills, SkillsEventCb, LV_EVENT_CLICKED, NULL);

    // Creates Left auton list
    leftlist = lv_list_create(main_screen);  
    lv_obj_set_size(leftlist, lv_pct(50), lv_pct(70));
    lv_obj_align(leftlist, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_set_style_pad_row(leftlist, 5, 0);

    // Creates Right auton list
    rightlist = lv_list_create(main_screen);  
    lv_obj_set_size(rightlist, lv_pct(50), lv_pct(70));
    lv_obj_align(rightlist, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_set_style_pad_row(rightlist, 5, 0);

    // Creates Skills auton list
    skillslist = lv_list_create(main_screen);  
    lv_obj_set_size(skillslist, lv_pct(50), lv_pct(70));
    lv_obj_align(skillslist, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_set_style_pad_row(skillslist, 5, 0);

    lv_obj_add_flag(rightlist, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(skillslist, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(main_screen, LV_OBJ_FLAG_SCROLLABLE);


    /*-----------------------------------------------------------------------------------------------------------------------------------
     █████╗ ██╗   ██╗████████╗ ██████╗ ███╗   ██╗    ██████╗ ██╗   ██╗████████╗████████╗ ██████╗ ███╗   ██╗███████╗
    ██╔══██╗██║   ██║╚══██╔══╝██╔═══██╗████╗  ██║    ██╔══██╗██║   ██║╚══██╔══╝╚══██╔══╝██╔═══██╗████╗  ██║██╔════╝
    ███████║██║   ██║   ██║   ██║   ██║██╔██╗ ██║    ██████╔╝██║   ██║   ██║      ██║   ██║   ██║██╔██╗ ██║███████╗
    ██╔══██║██║   ██║   ██║   ██║   ██║██║╚██╗██║    ██╔══██╗██║   ██║   ██║      ██║   ██║   ██║██║╚██╗██║╚════██║
    ██║  ██║╚██████╔╝   ██║   ╚██████╔╝██║ ╚████║    ██████╔╝╚██████╔╝   ██║      ██║   ╚██████╔╝██║ ╚████║███████║
    ╚═╝  ╚═╝ ╚═════╝    ╚═╝    ╚═════╝ ╚═╝  ╚═══╝    ╚═════╝  ╚═════╝    ╚═╝      ╚═╝    ╚═════╝ ╚═╝  ╚═══╝╚══════╝                        */
    create_btn(leftlist, "loader");
    create_btn(rightlist,"loader_right");
    create_btn(rightlist, "low_middle");
    create_btn(leftlist, "high_middle");
    create_btn(skillslist, "skills_2");
    // create_btn(rightlist, "wp");

    //-----------------------------------------------------------------------------------------------------------------------------------


    //Simple Image On Screen

    // LV_IMG_DECLARE(image);
    // lv_obj_t* img = lv_img_create(lv_scr_act());  // Create an image object on the active screen
    // lv_img_set_src(img, &image);                  // Set the image source to your declaleft image
    // lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
}

// Returns the user data of the currently selected button
const char* find_selected() {
    lv_obj_t* lists[] = { leftlist, rightlist, skillslist };
    for (int i = 0; i < 3; i++) {
        lv_obj_t* list = lists[i];
        uint16_t child_cnt = lv_obj_get_child_cnt(list);
        for (uint16_t idx = 0; idx < child_cnt; idx++) {
            lv_obj_t* btn = lv_obj_get_child(list, idx);
            if (lv_obj_has_state(btn, LV_STATE_CHECKED)) {
                // Return user data
                return static_cast<const char*>(lv_obj_get_user_data(btn));
            }
        }
    }
    // No button was checked
    return "auton_1";
}