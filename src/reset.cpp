#include "main.cpp"
#include "config.h"
#include "config.cpp"
#include <sys/_intsup.h>
#include "reset.h"
#include "pros/distance.hpp"

void distance_reset()
{   int front1_offset = 0; //offset for front sensor 1 in inches
    int front2_offset = 0; //offset for front sensor 2 in inches
    int side_offset1 = 0; //offset for side sensor in inches
    int side_offset2 = 0; //offset for side sensor in inches
    int field_size = 144; //field size in inches, used for calculating position when facing certain directions
    int heading = imu.get_rotation();
    double front1 = (front_sensor1.get_distance() / 25.4) + front1_offset; //convert to inches and add offset
    double front2 = (front_sensor2.get_distance() / 25.4) + front2_offset; //convert to inches and add offset
    double side_distance1 = (side_sensor1.get_distance() / 25.4) + side_offset1; //convert to inches and add offset
    double side_distance2 = (side_sensor2.get_distance() / 25.4) + side_offset2; //convert to inches and add offset
    int front_distance = (front1 + front2) / 2; //average the two front sensors for better accuracy
    
    if (355 <= heading && heading <= 5){
        chassis.setPose(side_distance1, front_distance, heading);
    }

    else if (85 <= heading && heading <= 95){
        
        chassis.setPose(front_distance, side_distance1, heading);
    }

    else if (175 <= heading && heading <= 185){
        
        chassis.setPose(side_distance1, front_distance, heading);
    }

    else if (265 <= heading && heading <= 275){
        
        chassis.setPose(front_distance, side_distance1, heading);
    }

    else {
        
        controller.print(0,0,"Heading not cardinal, cannot reset");
    }
}

