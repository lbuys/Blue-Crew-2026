#include "auto.cpp"



extern void place matchload(){
    controller.print(0, 0, "Place matchload             ");
    chassis.setPose(0, 0, 0);
    //drive to goal
    chassis.moveToPoint(-5,18,2000);
    liftStage = 1; 
    score();
    
    }
