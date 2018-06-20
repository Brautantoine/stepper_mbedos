#include "mbed.h"
#include "stepper.hpp"

///*** PROGRAMME DE TEST ***///
//**   Using MBED OS 5     **//
/*     TARGET : F767ZI       */

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
Ticker blink;
InterruptIn bp(USER_BUTTON);
Stepper stepper1(PA_3,PC_0);

void blink_led(void)
{
    led2=!led2;
    stepper1.kill();
}


int main() {
    bp.rise(blink_led);
    stepper1.move(400);
    stepper1.move(800);
    bool moved(false);
    int new_acc(0);
    int new_step(0);
    int new_dir(0);
    int new_vit(0);
    
    while(1)
    {
        led3=!led3;
        if(!stepper1.is_running()&&!moved){stepper1.move(-400);moved=true;}
        if(!stepper1.is_running())
        {
            printf("%c\033[2Jnouvelle commande stepper 1 : \n acc : ",7);
            scanf("%d",&new_acc);
            stepper1.set_accel(new_acc);
            printf("\nvitesse : ");
            scanf("%d",&new_vit);
            stepper1.set_rpm(new_vit);
            printf("\nmouvement : ");
            scanf("%d",&new_step);
            printf("\ndirection : ");
            scanf("%d",&new_dir);
            if(new_dir)stepper1.move(new_step);
            else stepper1.move(-new_step);
            printf("lancement ...\n");
        }
        wait_ms(200);
    }
}

