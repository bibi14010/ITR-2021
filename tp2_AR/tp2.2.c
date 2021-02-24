#include "tpl_os.h"
// Don't forget to delete before uploading into the robot
#include "../ecrobot_emul.h"

#define BACKWARD_SPEED -50
int main(void)
{
    StartOS(OSDEFAULTAPPMODE);
    return 0;
}
//----------------------- Declararions ------------------------
// Needed global varibles
int distance_value  = 0;
int speed           = 50;
int periode         = 0;
// Critical sections
DeclareResource(mutex);
//-------------------------------------------------------------
int PreTaskHook(void)
{
    TaskType id;
    GetTaskID(&id);
    return 0;
}
int PostTaskHook(void)
{
    TaskType id;
    GetTaskID(&id);
    return 0;
}
int StartupHook(void)
{
    // Init sensor on Port 3
    ecrobot_init_sonar_sensor(NXT_PORT_S3);
    // Init motor on port A & B -> go forward
    ecrobot_set_motor_speed(NXT_PORT_A,speed);
    ecrobot_set_motor_speed(NXT_PORT_A,speed);
    // Clean LCD
    display_update();
}
int ShutdownHook(void)
{
    //Terminate sensor communication
    ecrobot_term_sonar_sensor(NXT_PORT_S3);
    // Make the robot stop
    ecrobot_set_motor_speed(NXT_PORT_A,0);
    ecrobot_set_motor_speed(NXT_PORT_B,0);
}

TASK(contact)
{
    // Two sensors on Ports 1 and 2
    int sensor_one = ecrobot_get_touch_sensor(NXT_PORT_S1);
    int sensor_two = ecrobot_get_touch_sensor(NXT_PORT_S2);
    if(sensor_one | sensor_two ) {
        display_goto_xy(0,1);
        display_string("\n\rUpdating distance from touch sensor\n\r");
        display_update();
        GetResource(mutex);
        distance_value = 0 ;
        ReleaseResource(mutex);
    }
    TerminateTask();
}

TASK(distance)
{
    GetResource(mutex);
    display_goto_xy(0,1);
    display_string("\n\rUpdating distance from ultrasound sensor\n\r");
    display_update();
    distance_value = ecrobot_get_sonar_sensor(NXT_PORT_S3);
    TerminateTask();
}

TASK(navigation)
{
    GetResource(mutex);
    int distance = distance_value;
    ReleaseResource(mutex);
    // TODO
    if(distance == 0) {
        ecrobot_set_motor_speed(NXT_PORT_A,BACKWARD_SPEED);
        ecrobot_set_motor_speed(NXT_PORT_B,BACKWARD_SPEED);
    }else if(distance < 50 ) {
        // Turn around
    }else {
        if(periode >= 10){
            periode =0;
            // Turn around
        }
    }
    display_goto_xy(0,0);
    display_string("\n\rDISTANCE IS :\n\r");
    display_int(distance,3);
    display_update();
    TerminateTask();
}

TASK(stop)
{
  ShutdownOS(E_OK);
  TerminateTask();
}
