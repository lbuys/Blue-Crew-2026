#IMPORTS
import vex
import time
from vex import *
import math

#SET VARIABLE
brain = Brain()
front_left = Motor(Ports.PORT17, GearSetting.RATIO_6_1, False)
mid_left = Motor(Ports.PORT20, GearSetting.RATIO_6_1, False)
back_left = Motor(Ports.PORT18, GearSetting.RATIO_6_1, False)
front_right = Motor(Ports.PORT13, GearSetting.RATIO_6_1, True)
mid_right = Motor(Ports.PORT14, GearSetting.RATIO_6_1, True)
back_right = Motor(Ports.PORT16, GearSetting.RATIO_6_1, True)
intake_front = Motor(Ports.PORT10, GearSetting.RATIO_6_1, True)
intake_back = Motor(Ports.PORT21, GearSetting.RATIO_18_1, False)
con = Controller()
inertial = vex.Inertial(Ports.PORT19)
vertical_tracking = vex.Rotation(Ports.PORT15)
#horizontal_tracking = vex.Rotation(Ports.PORT11)

#Tuning Constants
kP_turn = 0.25
kD_turn = 0.25
kP_drive = 0.25
kD_drive = 0.25

# Exponential scaling for smoother control
# blend → blending factor for exponential part range of 0-1, 0=linear, 1=exponential
# base → exponential base slope of the exponential decrease if high jump at the top
# linear → linear part to keep low speed smooth 
# adjust blend → base → linear
blend = 0.8
base = 1.04
linear = 0.2

def pre_auton():
    # Calibrate inertial
    inertial.calibrate()
    while inertial.is_calibrating():
        wait(50, MSEC)  # wait until calibration is done
    con.rumble
    

    
    # AUTO SELECTOR
    def replace_green_box(auton_number):
        if auton_number == 1:
            brain.screen.set_fill_color(Color("#E8F1F2"))
            brain.screen.draw_rectangle(176, 20, 53, 53)
        elif auton_number == 2:
            brain.screen.set_fill_color(Color("#1B98E0"))
            brain.screen.draw_rectangle(405, 20, 53, 53)
        elif auton_number == 3:
            brain.screen.set_fill_color(Color("#247BA0"))
            brain.screen.draw_rectangle(176, 93, 53, 53)
        elif auton_number == 4:
            brain.screen.set_fill_color(Color("#006494"))
            brain.screen.draw_rectangle(405, 93, 53, 53)
        elif auton_number == 5:
            brain.screen.set_fill_color(Color("#13293D"))
            brain.screen.draw_rectangle(176, 166, 53, 53)
        elif auton_number == 6:
            brain.screen.set_fill_color(Color("#C5E7E2"))
            brain.screen.draw_rectangle(405, 166, 53, 53)
    # █████╗ ██╗   ██╗████████╗ ██████╗ ███╗   ██╗    ██████╗ ██╗   ██╗████████╗████████╗ ██████╗ ███╗   ██╗███████╗
    #██╔══██╗██║   ██║╚══██╔══╝██╔═══██╗████╗  ██║    ██╔══██╗██║   ██║╚══██╔══╝╚══██╔══╝██╔═══██╗████╗  ██║██╔════╝
    #███████║██║   ██║   ██║   ██║   ██║██╔██╗ ██║    ██████╔╝██║   ██║   ██║      ██║   ██║   ██║██╔██╗ ██║███████╗
    #██╔══██║██║   ██║   ██║   ██║   ██║██║╚██╗██║    ██╔══██╗██║   ██║   ██║      ██║   ██║   ██║██║╚██╗██║╚════██║
    #██║  ██║╚██████╔╝   ██║   ╚██████╔╝██║ ╚████║    ██████╔╝╚██████╔╝   ██║      ██║   ╚██████╔╝██║ ╚████║███████║
    #╚═╝  ╚═╝ ╚═════╝    ╚═╝    ╚═════╝ ╚═╝  ╚═══╝    ╚═════╝  ╚═════╝    ╚═╝      ╚═╝    ╚═════╝ ╚═╝  ╚═══╝╚══════╝   
    # Draw the first row of auton choices
    brain.screen.set_fill_color(Color("#E8F1F2"))
    brain.screen.draw_rectangle(20, 20, 209, 53)  # Auton 1
    brain.screen.draw_rectangle(176, 20, 53, 53)
    brain.screen.set_cursor(3, 5)
    brain.screen.print("Get Points")

    brain.screen.set_fill_color(Color("#1B98E0"))
    brain.screen.draw_rectangle(249, 20, 209, 53)  # Auton 2
    brain.screen.draw_rectangle(405, 20, 53, 53)
    brain.screen.set_cursor(3, 28)
    brain.screen.print("Lateral Tuning")

    brain.screen.set_fill_color(Color("#247BA0"))
    brain.screen.draw_rectangle(20, 93, 209, 53)  # Auton 3
    brain.screen.draw_rectangle(176, 93, 53, 53)
    brain.screen.set_cursor(6, 5)
    brain.screen.print("Angular Tuning")

    # Draw the second row of auton choices
    brain.screen.set_fill_color(Color("#006494"))
    brain.screen.draw_rectangle(249, 93, 209, 53)  # Auton 4
    brain.screen.draw_rectangle(405, 93, 53, 53)
    brain.screen.set_cursor(6, 28)
    brain.screen.print("Blank")

    brain.screen.set_fill_color(Color("#13293D"))
    brain.screen.draw_rectangle(20, 166, 209, 53)  # Auton 5
    brain.screen.draw_rectangle(176, 166, 53, 53)
    brain.screen.set_cursor(10, 5)
    brain.screen.print("Blank")

    brain.screen.set_fill_color(Color("#C5E7E2"))
    brain.screen.draw_rectangle(249, 166, 209, 53)  # Auton 6
    brain.screen.draw_rectangle(405, 166, 53, 53)
    brain.screen.set_cursor(10, 28)
    brain.screen.print("Blank")
    global selected_auton
    selected_auton = 1
    
    while True:
        if brain.screen.pressing():
            # Get the x and y positions of the screen press
            xscreen = brain.screen.x_position()
            yscreen = brain.screen.y_position()

            # Check which box was pressed
            global selected_auton
            
            replace_green_box(selected_auton)
            brain.screen.set_fill_color(Color.GREEN)
            if 20 <= xscreen <= 229 and 20 <= yscreen <= 73:  # Auton 1
                selected_auton = 1
                brain.screen.draw_rectangle(176, 20, 53, 53)
                #print("Auton 1 selected!")
            elif 249 <= xscreen <= 458 and 20 <= yscreen <= 73:  # Auton 2
                selected_auton = 2
                brain.screen.draw_rectangle(405, 20, 53, 53)
                #print("Auton 2 selected!")
            elif 20 <= xscreen <= 229 and 93 <= yscreen <= 146:  # Auton 3
                selected_auton = 3
                brain.screen.draw_rectangle(176, 93, 53, 53)
                #print("Auton 3 selected!")
            elif 249 <= xscreen <= 458 and 93 <= yscreen <= 146:  # Auton 4
                selected_auton = 4
                brain.screen.draw_rectangle(405, 93, 53, 53)
                #print("Auton 4 selected!")
            elif 20 <= xscreen <= 229 and 166 <= yscreen <= 219:  # Auton 5
                selected_auton = 5
                brain.screen.draw_rectangle(176, 166, 53, 53)
                #print("Auton 5 selected!")
            elif 249 <= xscreen <= 458 and 166 <= yscreen <= 219:  # Auton 6
                selected_auton = 6
                brain.screen.draw_rectangle(405, 166, 53, 53)
                #print("Auton 6 selected!")




    while True:
        if intake_direction == "out":
            intake_front.set_velocity(600, RPM) 
            intake_front.spin(REVERSE)
            intake_back.set_velocity(600,RPM)
            intake_back.spin(REVERSE)
    
        elif intake_direction == "in":
            intake_front.set_velocity(600, RPM)
            intake_front.spin(FORWARD)
            intake_back.set_velocity(600,RPM)
            intake_back.spin(FORWARD)

        elif intake_direction == "score":
            intake_front.set_velocity(600,RPM)
            intake_front.spin(FORWARD)
            intake_back.set_velocity(600,RPM)
            intake_back.spin(FORWARD)

        elif intake_direction == "Score_Slow":
            string.open()
            intake_front.set_velocity(400,RPM)
            intake_front.spin(FORWARD)
            intake_back.set_velocity(400,RPM)
            intake_back.spin(FORWARD)
        elif intake_direction == "stop":
            intake_front.set_stopping(BrakeType.COAST)
            intake_back.set_stopping(BrakeType.COAST)
            string.close
        wait(50, MSEC)


# EXPONENTIAL DRIVER CONTROL
def exponential_drive(input_value):

    if input_value == 0:
        return 0
    output = (input_value/ abs(input_value)) * (blend * (base ** abs(input_value) -1) + linear * abs (input_value))
    
    return max(-100, min(100, output)) 

from vex import *


 



# ██████╗ ██████╗ ██╗██╗   ██╗███████╗██████╗      ██████╗ ██████╗ ███╗   ██╗████████╗██████╗  ██████╗ ██╗     
 #██╔══██╗██╔══██╗██║██║   ██║██╔════╝██╔══██╗    ██╔════╝██╔═══██╗████╗  ██║╚══██╔══╝██╔══██╗██╔═══██╗██║     
 #██║  ██║██████╔╝██║██║   ██║█████╗  ██████╔╝    ██║     ██║   ██║██╔██╗ ██║   ██║   ██████╔╝██║   ██║██║     
 #██║  ██║██╔══██╗██║╚██╗ ██╔╝██╔══╝  ██╔══██╗    ██║     ██║   ██║██║╚██╗██║   ██║   ██╔══██╗██║   ██║██║     
 #██████╔╝██║  ██║██║ ╚████╔╝ ███████╗██║  ██║    ╚██████╗╚██████╔╝██║ ╚████║   ██║   ██║  ██║╚██████╔╝███████╗
 #╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═══╝  ╚══════╝╚═╝  ╚═╝     ╚═════╝ ╚═════╝ ╚═╝  ╚═══╝   ╚═╝   ╚═╝  ╚═╝ ╚═════╝ ╚══════╝ 
def drivercontrol():
    
    front_right.set_stopping(BrakeType.COAST)
    mid_right.set_stopping(BrakeType.COAST)
    back_right.set_stopping(BrakeType.COAST)
    back_left.set_stopping(BrakeType.COAST)
    mid_left.set_stopping(BrakeType.COAST)
    front_left.set_stopping(BrakeType.COAST)
    
    while True:
        # Apply exponential scaling to joystick inputs
        axis3_value = exponential_drive(con.axis3.position())
        axis1_value = (exponential_drive(con.axis1.position()))*.8

        # Driving control with exponential adjustment
        front_right.spin(FORWARD, (axis3_value + axis1_value), PERCENT)
        mid_right.spin(FORWARD, (axis3_value + axis1_value), PERCENT)
        back_right.spin(FORWARD, (axis3_value + axis1_value), PERCENT)
        front_left.spin(FORWARD, (axis3_value - axis1_value), PERCENT)
        mid_left.spin(FORWARD, (axis3_value - axis1_value), PERCENT)
        back_left.spin(FORWARD, (axis3_value - axis1_value), PERCENT)

        con.screen.print(axis3_value)
        
        

# Autonomous controls
global_x = 0.0
global_y = 0.0
previous_position = 0.0
previous_heading = 0.0
previous_heading_error = 0.0
dt = 0.02
previous_distance_to_target = 0.0

def update_position():
    global global_x, global_y, previous_position, previous_heading
    

    vertical_offset_distance = 0.4375
    vertical_wheel_diameter = 2
    vertical_wheel_circumference = vertical_wheel_diameter * math.pi

    #Encoder Position (inches)
    current_position = (vertical_tracking.position(DEGREES))*(vertical_wheel_circumference/360)

    #Current Heading (Radians)
    current_heading = math.radians(inertial.heading(DEGREES))

    #Change in Heading since last cycle
    delta_heading = current_heading - previous_heading

    #Normalize Heading to [-pi,pi]
    if delta_heading > math.pi:
        delta_heading -= 2 * math.pi
    elif delta_heading < -math.pi:
        delta_heading += 2 * math.pi

    #Real position after offset distance factored
    actual_position = current_position - delta_heading * vertical_offset_distance
    
    delta_position = actual_position - previous_position

    distance_moved = delta_position
               
    #calculate movement 
    if abs(delta_heading) < 1e-5:  #straight line
        delta_x = distance_moved * math.cos(current_heading)
        delta_y = distance_moved * math.sin(current_heading)
    else:  #robot is turning
        #calculate radius of the path
        radius = distance_moved / delta_heading
        
        #find the center
        delta_x = radius * (math.sin(current_heading) - math.sin(previous_heading))
        delta_y = -radius * (math.cos(current_heading) - math.cos(previous_heading))
    
    #update global position
    global_x += delta_x
    global_y += delta_y
    
    #save current encoder positions and heading for next run through
    previous_position = actual_position
    previous_heading = current_heading


def Move_to_heading(target_x, target_y, target_heading, exit_range = 5 , timeout=4000):




    start_time = brain.timer.time()  # Record start time for timeout tracking
    while True:

        update_position()

         # Calculate deltas to the current target point
        delta_x = target_x - global_x 
        delta_y = target_y - global_y

        distance_to_target = (delta_x ** 2 + delta_y ** 2) ** 0.5

        current_heading = inertial.heading(DEGREES)
        
        
        delta_heading = target_heading - current_heading
        if delta_heading > 180:
            delta_heading -= 360
        elif delta_heading < -180:
            delta_heading += 360

        turn_derivative =  (delta_heading - previous_heading_error) / dt
        lateral_derivative = (distance_to_target - previous_distance_to_target) / dt
        # clamp lateral derivative for better results
        lateral_derivative1 = max(min(lateral_derivative, 50), -50)

        # Check if within lookAhead distance
        if distance_to_target <= exit_range:

            front_right.stop(HOLD)
            mid_right.stop(HOLD)
            back_right.stop(HOLD)
            front_left.stop(HOLD)
            mid_left.stop(HOLD)
            back_left.stop(HOLD)

            break # End loop to move to next function
                


            # Check timeout
        time_elapsed = brain.timer.time() - start_time
        if time_elapsed > timeout:
                break

            # Calculate turn speed based on heading error
        turn_speed = kP_turn * delta_heading + kD_turn * turn_derivative
        lateral_speed = kP_drive * distance_to_target + kD_drive * lateral_derivative1
            # Calculate motor speeds
        left_speed = lateral_speed + turn_speed
        right_speed = lateral_speed - turn_speed

            # Clamp motor speeds to [-100, 100]
        left_speed1 = max(min(left_speed, 100), -100)
        right_speed1 = max(min(right_speed, 100), -100)

            # Set motor speeds
        front_left.spin(FORWARD, left_speed1, PERCENT)
        mid_left.spin(FORWARD, left_speed1, PERCENT)
        back_left.spin(FORWARD, left_speed1, PERCENT)
        front_right.spin(FORWARD, right_speed1, PERCENT)
        mid_right.spin(FORWARD, right_speed1, PERCENT)
        back_right.spin(FORWARD, right_speed1, PERCENT)

        previous_heading_error= delta_heading
        previous_distance_to_target = distance_to_target

            # Small delay for loop stability
        wait(20, MSEC)

    # Stop all motors at the end
    front_right.stop(HOLD)
    mid_right.stop(HOLD)
    back_right.stop(HOLD)
    front_left.stop(HOLD)
    mid_left.stop(HOLD)
    back_left.stop(HOLD)





def Turn_To( target_heading , direction, exit_range = 1):

    
    while True:

        update_position()

        current_heading = inertial.heading(DEGREES)

        # Calculate the heading error (delta heading)
        delta_heading = target_heading - current_heading
        if delta_heading > 180:
            delta_heading -= 360
        elif delta_heading < -180:
            delta_heading += 360

        if delta_heading < exit_range:
                
            front_right.stop(HOLD)
            mid_right.stop(HOLD)
            back_right.stop(HOLD)
            front_left.stop(HOLD)
            mid_left.stop(HOLD)
            back_left.stop(HOLD)
            
            break # Go to the next function

            
        derivative =  (delta_heading - previous_heading_error) / dt

        turn_speed = kP_turn * delta_heading + kD_turn * derivative

        
        if direction.lower() == "left":
            front_left.spin(REVERSE, turn_speed, PERCENT)
            mid_left.spin(REVERSE, turn_speed, PERCENT)
            back_left.spin(REVERSE, turn_speed, PERCENT)
            front_right.spin(FORWARD, turn_speed, PERCENT)
            mid_right.spin(FORWARD, turn_speed, PERCENT)
            back_right.spin(FORWARD, turn_speed, PERCENT)
        elif direction.lower() == "right":
            front_left.spin(FORWARD, turn_speed, PERCENT)
            mid_left.spin(FORWARD, turn_speed, PERCENT)
            back_left.spin(FORWARD, turn_speed, PERCENT)
            front_right.spin(REVERSE, turn_speed, PERCENT)
            mid_right.spin(REVERSE, turn_speed, PERCENT)
            back_right.spin(REVERSE, turn_speed, PERCENT)



def print_pos():
    while True:
        update_position()
        brain.screen.clear_line(1)  
        brain.screen.set_cursor(1, 1)
        brain.screen.print("X: {:.2f}  Y: {:.2f}".format(global_x, global_y))
        wait(20, MSEC)
 
 
 # █████╗ ██╗   ██╗████████╗ ██████╗ ███╗   ██╗ ██████╗ ███╗   ███╗ ██████╗ ██╗   ██╗███████╗
 #██╔══██╗██║   ██║╚══██╔══╝██╔═══██╗████╗  ██║██╔═══██╗████╗ ████║██╔═══██╗██║   ██║██╔════╝
 #███████║██║   ██║   ██║   ██║   ██║██╔██╗ ██║██║   ██║██╔████╔██║██║   ██║██║   ██║███████╗
 #██╔══██║██║   ██║   ██║   ██║   ██║██║╚██╗██║██║   ██║██║╚██╔╝██║██║   ██║██║   ██║╚════██║
 #██║  ██║╚██████╔╝   ██║   ╚██████╔╝██║ ╚████║╚██████╔╝██║ ╚═╝ ██║╚██████╔╝╚██████╔╝███████║
 #╚═╝  ╚═╝ ╚═════╝    ╚═╝    ╚═════╝ ╚═╝  ╚═══╝ ╚═════╝ ╚═╝     ╚═╝ ╚═════╝  ╚═════╝ ╚══════╝
 
def autonomous():
    front_right.set_stopping(BrakeType.COAST)
    mid_right.set_stopping(BrakeType.COAST)
    back_right.set_stopping(BrakeType.COAST)
    back_left.set_stopping(BrakeType.COAST)
    mid_left.set_stopping(BrakeType.COAST)
    front_left.set_stopping(BrakeType.COAST)
    
    

    if selected_auton == 1: 
        while True:
            print_pos()

    if selected_auton == 2:
        #while True:
        Move_to_heading(40,0,0)
        print_pos()

    if selected_auton == 3:
        while True:
            Turn_To(90,"right")
            print_pos()


competition = Competition(drivercontrol,autonomous)

#run pre auton
pre_auton()


