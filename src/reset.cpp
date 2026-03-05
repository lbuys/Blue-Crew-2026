#include "main.cpp"
#include "config.h"
#include "config.cpp"
#include <sys/_intsup.h>
#include "reset.h"
#include "pros/distance.hpp"

void distance_reset()
{   int front1_offset = 0; //offset for front sensor 1 in inches
    int front2_offset = 0; //offset for front sensor 2 in inches
    int side_offset = 0; //offset for side sensor in inches
    int heading = imu.get_rotation();
    int front1 = (front_sensor1.get_distance() / 25.4) + front1_offset; //convert to inches and add offset
    int front2 = (front_sensor2.get_distance() / 25.4) + front2_offset; //convert to inches and add offset
    int side_distance = (side_sensor.get_distance() / 25.4) + side_offset; //convert to inches and add offset
    int front_distance = (front1 + front2) / 2.0;
    
    if (-5 <= heading && heading <= 5){
        chassis.setPose(side_distance, front_distance, chassis.getPose().theta);
    }

    else if (85 <= heading && heading <= 95){
        
        chassis.setPose(front_distance, side_distance, chassis.getPose().theta);
    }
    
    else if (175 <= heading && heading <= 185){
        
        chassis.setPose(side_distance, front_distance, chassis.getPose().theta);
    }

    else if (265 <= heading && heading <= 275){
        
        chassis.setPose(front_distance, side_distance, chassis.getPose().theta);
    }

    else {
        
        controller.print(0,0,"Heading not cardinal, cannot reset");
    }
}

