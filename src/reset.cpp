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
    
    if (-5 <= heading && heading <= 5){
        chassis.setPose(side_distance, front_distance, chassis.getPose().theta);

}
    if (85 <= heading && heading <= 95){
        
        chassis.setPose(side_distance, front_distance, chassis.getPose().theta);

}
    else if (175 <= heading && heading <= 185){
        
        chassis.setPose(side_distance, front_distance, chassis.getPose().theta);}

    else if (265 <= heading && heading <= 275){
        
        chassis.setPose(side_distance, front_distance, chassis.getPose().theta);}

    else {
        
        controller.print(0,0,"Heading not cardinal, cannot reset");
    
    }
}

