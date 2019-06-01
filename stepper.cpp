#include "stepper.hpp"

int Stepper::nb_stepper=0;                                                      // initialisation du nombre de stepper (requis)

/// Constructeur ///
//
//
//
///             ///
Stepper::Stepper(DigitalInOut n_step,DigitalInOut n_dir):step_pin(n_step),dir_pin(n_dir)
{
    /// Passage des pins en sortie ///
    step_pin.output();
    dir_pin.output();
    
    /// Passage des pins en drain ouvert ///
    step_pin.mode(OpenDrain);
    dir_pin.mode(OpenDrain);
    
    /// settings par défaut ///
    direction=true;
    run=false;
    step_delay=1;
    step_required=0;
    step=0;
    pos=0;
    mode=PROTECTED;
    
    rpm=300;
    acc=200;
    c0=0.676f*(float)FREQ*sqrt((2*ANGLE)/(float)acc);
    
    thread_is_running=true;
    step_thread.start(callback(this,&Stepper::make_step));
    
    nb_stepper++;
       
}
/// Destructeur ///
//
//
//
///             ///
Stepper::~Stepper()
{
    this->kill();
    thread_is_running=false;
    step_thread.join();
    nb_stepper--;
}
///   kill     ///
//  Fonction d'arret du stepper
//
///            ///
void Stepper::kill()
{
    step_required=0;
    run=false;
}
///   move     ///
// commande un nombre de step au stepper
// \param : nb_step -> indique le nombre de step, un nombre négatif produit un mouvement dans l'autre sens
// \return : 0 -> commande accepté, 1 -> commande refusé
///            ///
int Stepper::move(int nb_step)
{
    int ret(0);

    if(!run||mode){
        /*** mise à jour de la direction ***/
        if(nb_step>=0)
        {
            direction=true;
            dir_pin=1;
            step_required=nb_step;
        }
        else
        {
            direction=false;
            dir_pin=0;
            step_required=-nb_step;
        }
        #ifdef DEBUG_VERBOSE
        printf("nouvelle commande, dir : %d, step : %-d\n",direction,step_required); // DEBUG
        #endif
       
       /*** lancement du mouvement ***/ 
        run=true;
        
    }
    else ret=-1;
    return ret;
    
}
///  make_step  ///
// Fonction de génération des steps
// S'execute sur avec un thread secondaire
//
///             ///
void Stepper::make_step()
{
    float clast=c0;
    int step_middle=step_required/2;
    uint8_t state=FIRST_STEP;
    float elapsed_time;
    int elapsed_step;
    /*** debut de boucle ***/
    while(thread_is_running){
        if(run){
            #ifdef DEBUG_VERBOSE
            printf("debut de commande\n");                                      // DEBUG
            #endif
            #ifdef DEBUG
            led1=!led1;                                                         //DEBUG
            #endif
            clast=c0;
            step=0;
            step_middle=step_required/2;
            state=FIRST_STEP;
            elapsed_time=0;
            elapsed_step=0;
            /*** generation des steps ***/
            while(step<step_required)
            {
                step++;
                if(direction)pos++;
                else pos--;
                switch(state)
                {
                    case FIRST_STEP:
                        step_pin=1;
                        wait_us(20);
                        step_pin=0;
                        step_delay=clast-((2.f*clast)/((4.f*(float)step)+1.f));       // Formule aproximative
                        state=ACC;
                        break;
                    case ACC:
                        step_pin=1;
                        wait_us(20);
                        step_pin=0;
                        step_delay=clast-((2.f*clast)/((4.f*(float)step)+1.f));
                        if(step>=step_middle)state=DEC;
                        else if(acc*elapsed_time>=(float)rpm)
                        {
                            elapsed_step=step;
                            state=CONST;
                        }
                        break;
                    case DEC:
                        step_pin=1;
                        wait_us(20);
                        step_pin=0;
                        step_delay=clast-((2.f*clast)/((4.f*((float)step_middle-(float)step)*4.f)+1.f)); // Le *4.f sort de nul part mais ça marche bien
                        break;
                    case CONST:
                        step_pin=1;
                        wait_us(20);
                        step_pin=0;
                        if(step>=step_required-elapsed_step)state=DEC;
                        break;
                }
                clast=step_delay;
                Thread::wait(uint32_t(step_delay*1000));                        
                elapsed_time+=step_delay;
            }
            run=false;
            #ifdef DEBUG
            led1=!led1;                                                         // DEBUG
            #endif
            #ifdef DEBUG_VERBOSE
            printf("fin de commande\n");                                        // DEBUG
            #endif
        }
    
    }
}
