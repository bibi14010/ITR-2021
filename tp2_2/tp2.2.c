#include <stdio.h>
#include "tpl_os.h"
#include <time.h>
#include <string.h>
#include "../ecrobot_emul.h"
#include <stdbool.h>
#include <sys/time.h>
#include <stdlib.h>
#define BACKWARD_SPEED -10
int main(void)
{
    StartOS(OSDEFAULTAPPMODE);
    return 0;
}
//################# Declare Global Variable #################
int distance_value  = 0;
int periode         = 0;
int degree          = 30;
int speed           = 0;
struct timespec current_time;
//int current_time  = 0;
//############################################################
int PreTaskHook(void)
{
    TaskType id;
    GetTaskID(&id);
    printf("\r\nTask%d : Pretaskhook\r\n",id);
    return 0;
}
int PostTaskHook(void)
{
    TaskType id;
    GetTaskID(&id);
    printf("\r\nTask%d: Posttaskhook\n\r",id);
    return 0;
}
int StartupHook(void)
{
    // Init sensor on Port 2
    ecrobot_init_sonar_sensor(NXT_PORT_S2);
    // Init motor on port A -> go forward
    ecrobot_set_motor_speed(NXT_PORT_A,speed);
    // Declare alarms
    SetRelAlarm(contact_alarm,10,100);
    SetRelAlarm(distance_alarm,10,300);
    SetRelAlarm(navigation_alarm,10,200);
}
int ShutdownHook(void)
{
    //Terminate sensor communication
    ecrobot_term_sonar_sensor(NXT_PORT_S2);
    // Make the robot stop
    ecrobot_set_motor_speed(NXT_PORT_A,0);
}

TASK(contact)
{
    // Check contact
    // Pare-choc sur port 1
    printf("\r\n========TASK CONTACT========\r\n");
    if (ecrobot_get_touch_sensor(NXT_PORT_S1) == 1)
    {
        printf("\r\nFRONT TOUCH : YES\r\n");
        GetResource(mutex);
        distance_value = 0;
        ReleaseResource(mutex);
    }else{
         printf("\r\nFRONT TOUCH : NO\r\n");
    }
    TerminateTask();
}

TASK(distance)
{
    printf("\r\n========TASK DISTANCE========\r\n");
    // Display time and distance
    clock_gettime(CLOCK_MONOTONIC_RAW,&current_time);
    //current_time = ecrobot_get_systick_ms();
    GetResource(mutex);
    unsigned long tmp = current_time.tv_sec*1000000+current_time.tv_nsec/1000;
    printf("\r\n%lu Distance = %d\r\n",tmp,distance_value);
    // Ultrasons sur port 2
    distance_value = ecrobot_get_sonar_sensor(NXT_PORT_S2);
    ReleaseResource(mutex);
    TerminateTask();
}

TASK(navigation)
{
    //move
    periode++;
    printf("\r\n========TASK NAVIGATION========\r\n");
    GetResource(mutex);
    // NXT_PORT_A
    if(distance_value == 0){
        // Go backward
        ecrobot_set_motor_speed(NXT_PORT_A,BACKWARD_SPEED);
    }else if (distance_value <50){
        // Turn
        // TODO mieux gérer les virages
        nxt_motor_set_count(NXT_PORT_A,45);
    } else {
        nxt_motor_set_count(NXT_PORT_A,0);
    }
    if(speed<100){
        // Speed ++
        speed+=10;
        ecrobot_set_motor_speed(NXT_PORT_A,speed);
    }
    if (periode % 10 ==0){
        // Turn
        degree = random() % 180 ;
        nxt_motor_set_count(NXT_PORT_A,degree);
    }
    ReleaseResource(mutex);
    TerminateTask();
}

TASK(stop)
{
  ShutdownOS(E_OK);
  TerminateTask();
}
