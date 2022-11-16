#include "modules.h"


#define TARGET_VELOCITY 25.0

#define KP  400// 200
#define KI  200// 100
#define KD 0 // 30
#define DIV  1
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

volatile int sum_error=0 ;
volatile short int prev_error=0;

void straight_PID(){
    int error = leftWheelCount - rightWheelCount;
    clearLeftWheelCount();
    clearRightWheelCount();
    printf("\nerror : %d", error);

    sum_error += error;
    printf(" sum_error : %d", sum_error);
    int adjust = (KP * error) + (KI * sum_error) + (KD * (error -prev_error) ) ;
    prev_error = error;
    //printf(" used PWM: %d ",adjust);

    int l_used_PWM  = pwmConfig2.dutyCycle -  adjust / 2 ;
    int r_used_PWM  =  pwmConfig.dutyCycle +  adjust / 2  ;

    printf(" used PWM: %d ",l_used_PWM);
    if (motor_state =='0'){
        pwmConfig.dutyCycle = 0;
        pwmConfig2.dutyCycle = 0;
    }

    pwmConfig2.dutyCycle = MAX(MIN(10000,l_used_PWM),0);
    pwmConfig.dutyCycle = MAX(MIN(10000,r_used_PWM),0);

    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig2);
    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);
}
/*
volatile float sum_left_error = 0.0;
volatile float sum_right_error= 0.0;
volatile float prev_left_error = 0.0;
volatile float prev_right_error= 0.0;




void pid_Left_Straight(){
    float velocity =  wheel_Left_Velocity;
    float new_error =  TARGET_VELOCITY - velocity ;
    sum_left_error +=new_error;
    float final_error = new_error * KP +  sum_left_error * KI  + (new_error - prev_left_error) * KD;
    prev_left_error = new_error ;

    uint32_t used_PWM = pwmConfig2.dutyCycle + pwmConfig2.dutyCycle * (final_error / velocity) ;
    pwmConfig2.dutyCycle = (uint32_t)MAX(MIN(9000,used_PWM),3000);
    //sum_left_error += new_error;
    if (motor_state =='0'){
        pwmConfig2.dutyCycle =0;
    }

    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig2);
}

void pid_Left_Straight(){
    float velocity = wheel_Left_Velocity;
    float new_error =  TARGET_VELOCITY - velocity ;


    uint32_t used_PWM = pwmConfig2.dutyCycle + pwmConfig2.dutyCycle * (new_error / velocity * KP) ;
    pwmConfig2.dutyCycle = (uint32_t)MAX(MIN(9000,used_PWM),3000);
    //sum_left_error += new_error;
    prev_left_error = new_error ;
    if (motor_state =='0'){
        pwmConfig2.dutyCycle =0;
    }

    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig2);

}


void pid_Right_Straight(){
    float velocity =  wheel_Right_Velocity;
    float new_error =  TARGET_VELOCITY - velocity ;
    sum_right_error +=new_error;
    float final_error = new_error * KP +  sum_right_error * KI  + (new_error - prev_right_error) * KD;
    prev_right_error = new_error ;

    uint32_t used_PWM = pwmConfig.dutyCycle + pwmConfig.dutyCycle * (final_error / velocity) ;
    pwmConfig.dutyCycle = (uint32_t)MAX(MIN(9000,used_PWM),3000);
    //sum_right_error += new_error;
    if (motor_state =='0'){
        pwmConfig.dutyCycle = 0;
    }
    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);

}
*/



