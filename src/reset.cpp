#include "main.cpp"
#include "config.h"
#include "config.cpp"
#include "reset.h"

void distance_reset()
{   int heading = imu.get_rotation();
    int front1 = front_sensor1.get_distance();
    int front2 = front_sensor2.get_distance();
    int side_distance = side_sensor.get_distance();
    int front_distance = (front1 + front2) / 2.0;
    
    if (heading == 0){
        chassis.setPose(side_distance, front_distance, chassis.getPose().theta);

}
    if (heading == 90){
        
        chassis.setPose(side_distance, front_distance, chassis.getPose().theta);

}
    else if (heading == 180){
        
        chassis.setPose(side_distance, front_distance, chassis.getPose().theta);}

    else if (heading == 270){
        
        chassis.setPose(side_distance, front_distance, chassis.getPose().theta);}

    else {
        
        controller.print(0,0,"Heading not cardinal, cannot reset");
    
    }
}

