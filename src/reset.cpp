#include "main.cpp"
#include "config.h"
#include "config.cpp"
#include "reset.h"

void distance_reset()
{   int heading = imu.get_rotation();
    
    if (heading == 0){
        int x1 = x_sensor1.get_distance();
        int x2 = x_sensor2.get_distance();
        int y = y_sensor.get_distance();

        int x_distance = (x1 + x2) / 2.0;

        chassis.setPose(x_distance, y, chassis.getPose().theta);

}
    if (heading == 90){
        int x1 = x_sensor1.get_distance();
        int x2 = x_sensor2.get_distance();
        int y = y_sensor.get_distance();

        int x_distance = (x1 + x2) / 2.0;

        chassis.setPose(x_distance, y, chassis.getPose().theta);

}
    else if (heading == 180){
        int x1 = x_sensor1.get_distance();
        int x2 = x_sensor2.get_distance();
        int y = y_sensor.get_distance();

        int x_distance = (x1 + x2) / 2.0;

        chassis.setPose(x_distance, y, chassis.getPose().theta);}

    else if (heading == 270){
        int x1 = x_sensor1.get_distance();
        int x2 = x_sensor2.get_distance();
        int y = y_sensor.get_distance();

        int x_distance = (x1 + x2) / 2.0;

        chassis.setPose(x_distance, y, chassis.getPose().theta);}

    else {
        
        controller.print(0,0,"Heading not cardinal, cannot reset");
    
    }
}

