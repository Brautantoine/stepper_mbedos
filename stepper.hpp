//** Ajouter mbed-os au projet **//

/*******************************************************************************
*   MIT License
*
*   Copyright (c) 2018 Braut Antoine in the name of Square Food
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*******************************************************************************/

         //***   **Version 1.3**  **Revisions 08/09/2018** ***//
         
/*******************************************************************************
*   
*   Cette librairie décrit la class stepper, utilisée pour contrôler un
*   stepper avec un pin de pulse et de direction en drain ouvert.
*   L'acceleration est traité de maniére linéaire avec une gestion de la
*   vitesse maximum.
*   Matériel de test:
*   µc : nucleo-144 F767ZI
*   stepper : nema 23 57HS76 2 phase hybrid stepper
*   Contrôleur de stepper : MB4550A
*
*******************************************************************************/


#ifndef STEPPER_HPP
#define STEPPER_HPP

#include "mbed.h"
#include "math.h"

/*** MACROS D'ETAT ***/
#define FIRST_STEP 0
#define ACC 1
#define DEC 2
#define CONST 3
/*** MACROS DE MODE ***/
#define OVERWRITE true //une nouvelle commande peut être accepté avant la fin de la premiére
#define PROTECTED false //Toutes nouvelles commandes est ignoré tant qu'une est active
/*** MACROS DE DEBUG - DECOMMENTER POUR ACTIVER ***/
//#define DEBUG
//#define DEBUG_VERBOSE

#ifndef DEBUG
extern DigitalOut led1; //Debug led <- main.cpp
#endif

class Stepper
{
    public:
    /*** Constructeur et destructeur ***/
    Stepper(DigitalInOut n_step,DigitalInOut n_dir);                            //Constructeur
    ~Stepper();                                                                 //Destructeur
    /*** Getter d'etat ***/
    bool is_running() {return run;}                                             //Renvoie si le moteur effectue une commande ou non
    int get_pos() {return pos;}                                                 //Renvoie la position du stepper
    /*** Modificateurs de paramètres ***/
    void set_mode(bool n_mode) {mode=n_mode;}                                   //Permet de changer le mode du stepper cf MACROS DE MODE
    void set_accel(unsigned int rpms)                                           //Modifie l'acceleration du stepper
        {acc=rpms;c0=0.676f*(float)FREQ*sqrt((2*ANGLE)/(float)acc);}            /** Recalcule le délai du premier step **/
    void set_rpm(unsigned int n_rpm) {rpm=n_rpm;}                               //Modifie la vitesse du stepper
    /*** Actionneur ***/
    int move(int nb_step);                                                      //Effectue le nombre de step demandé
    void kill();                                                                //Stop le stepper
    
    /*** Nombre de stepper actif ***/
    static int nb_stepper;
    
    private:
    
    void make_step();                                                           //Effectue le nombre de step demandé en controlant l'accélération (S'execute sur un thread secondaires tant que l'instance est présente) 
    
    /*** Stepper settings ***/
    DigitalInOut step_pin;                                                      // Pin effectuant les pulses de step
    DigitalInOut dir_pin;                                                       // Pin controlant la direction
    
    bool run;                                                                   // Indique si le stepper tourne ou non
    bool mode;                                                                  // Indique le mode de fonctionnement (cf MACRO DE MODE)
    
    int rpm;                                                                    // vitesse du stepper
    float acc;                                                                  // accélération du stepper
    float c0;                                                                   //délai initial
    
    int pos;                                                                    //position absolue en nombre de pas
    
    /*** mouvement settings ***/
    bool direction;                                                             // indique la direction du mouvement
    int step_required;                                                          // indique le nombre de step du mouvement 
    int step;                                                                   // indique le nombre de step effectué
    float  step_delay;                                                          // indique le delai avant le prochain step
    
    /*** Thread ***/
    Thread step_thread;                                                         // thread du génération de mouvement du stepper
    bool thread_is_running;                                                     // indique si le thread doit continuer ou non
    
    /*** Constante ***/
    static const float ANGLE=0.0157;                                            // angle d'un step en radian
    static const long FREQ=1*10^6;                                              // fréquence du timer (arbitraire)
};

#endif