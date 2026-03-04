#include "main.cpp"
#include "config.h"
#include "config.cpp"
#include "reset.h"

void distance_reset()
{   
    int x1 = x_sensor1.get_distance();
    int x2 = x_sensor2.get_distance();
    int y = y_sensor.get_distance();

    int x_distance = (x1 + x2) / 2.0;

    chassis.setPose(x_distance, y, chassis.getPose().theta);

    
}

