#include "auto.cpp"



extern void place matchload(){
    controller.print(0, 0, "Place matchload             ");
    chassis.setPose(0, 0, 0);
    //drive to goal
    chassis.moveToPose(-5,18,-48,2000,{.minSpeed=60,.earlyExitRange=8});
    lift_movement_up();//position0-8
    chassis.moveToPose(-50, 26.2, -180, 2000,{.minSpeed=60,.earlyExitRange=8});
    //Line up point
    chassis.moveToPoint(-39, 4.8, 2000,{.minSpeed=60,.earlyExitRange=8});
    chassis.turnToHeading(0, 1500,{.earlyExitRange=10});
    //Long goal
    chassis.moveToPose(-37.5, 20, 0, 1500,{},false);
    direction = IntakeDirection::Score;
    pros::delay(350);
    direction = IntakeDirection::Out;
    pros::delay(150);
    direction = IntakeDirection::Score;
    pros::delay(1200);
    //Backup
    chassis.moveToPose(-38, 5,0, 2000,{.forwards=false,.minSpeed=1,.earlyExitRange=5},false);
    willy.set_value(true);
    string.set_value(false);
    direction = IntakeDirection::In;
    chassis.turnToHeading(-180, 1500,{.earlyExitRange=10});
    //Loader
    chassis.moveToPose(-37, -10, -180, 1500,{.maxSpeed=90,.minSpeed=80,.earlyExitRange=3});
    chassis.waitUntilDone();
    chassis.cancelMotion();
    chassis.tank(30, 30);
    pros::delay(1000);
    chassis.tank(0, 0);
    //backup
    chassis.moveToPoint(-37, 2.5,1000,{.forwards=false,.minSpeed=60,.earlyExitRange=5});
    chassis.turnToHeading(45, 1000,{.minSpeed=30,.earlyExitRange=10});
    willy.set_value(false);
    //High middle goal
    lift.set_value(false);
    chassis.moveToPose(-7, 31, 45, 3000,{.maxSpeed=70,.minSpeed=30,.earlyExitRange=14},false);
    direction = IntakeDirection::Score;
    descore_bottom.set_value(true);
    descore.set_value(true);
    pros::delay(1500);
    //backup
    chassis.moveToPoint(-36, 6,1000,{.forwards=false,.minSpeed=80,.earlyExitRange=5});
    direction=IntakeDirection::Stop;
    chassis.turnToHeading(507, 1000,{.minSpeed=30,.earlyExitRange=10});
    //push
    chassis.moveToPose(-46.1, 30.5, 180, 1500,{.forwards=false,.minSpeed=60,.earlyExitRange=3}); 
}
