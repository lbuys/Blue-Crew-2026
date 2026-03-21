#include "main.cpp"
#include "config.h"
#include "config.cpp"
#include <sys/_intsup.h>
#include "reset.h"
#include "pros/distance.hpp"

    //Constants
    //offsets for calibrating distance sensors
    int front_offset = 0; //offset for front sensor 1 in inches
    int back_offset = 0; //offset for back sensor in inches
    int left_offset = 0; //offset for left sensor in inches
    int right_offset = 0; //offset for right sensor in inches

    int field_size = 144; //field size in inches, used for calculating position when facing certain directions
    
    
    void distance_reset() {
        //get distance readings from each sensor and convert to inches
        double front_distance = (front_sensor.get_distance() / 25.4) + front_offset; //convert to inches and add offset
        double back_distance = (back_sensor.get_distance() / 25.4) + back_offset; //convert to inches and add offset
        double left_distance = (left_sensor.get_distance() / 25.4) + left_offset; //convert to inches and add offset
        double right_distance = (right_sensor.get_distance() / 25.4) + right_offset; //convert to inches and add offset

        //get current rotation from IMU and convert to radians
        double theta_degrees = imu.get_rotation(); //get current rotation from IMU
        double theta_radians = theta_degrees * (M_PI / 180); //convert to radians
        double theta = theta_radians; //store in variable for easier use
        
        //calculate x and y position using distance sensor readings and current heading
        double x1 = (front_distance * cos(theta) + (field_size - right_distance) * sin(theta)); //calculate x position using front and right sensors
        double x2 = (front_distance * cos(theta) - left_distance * sin(theta)); //calculate x position using front and left sensors
        double x3 = (-(field_size - back_distance) * cos(theta) + (field_size - right_distance) * sin(theta)); //calculate x position using back and right sensors
        double x4 = (-(field_size - back_distance) * cos(theta) - left_distance * sin(theta)); //calculate x position using back and left sensors

        double x = (x1 + x2 + x3 + x4) / 4; //average x positions for more accuracy

        double y1 = (front_distance * sin(theta) - (field_size - right_distance) * cos(theta)); //calculate y position using front and right sensors
        double y2 = (front_distance * sin(theta) + left_distance * cos(theta)); //calculate y position using front and left sensors
        double y3 = (-(field_size - back_distance) * sin(theta) - (field_size - right_distance) * cos(theta)); //calculate y position using back and right sensors
        double y4 = (-(field_size - back_distance) * sin(theta) + left_distance * cos(theta)); //calculate y position using back and left sensors

        double y = (y1 + y2 + y3 + y4) / 4; //average y positions for more accuracy

    }

    


    