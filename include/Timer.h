/*

TITRE          : Feux de signalisation en POO.
AUTEUR         : Franck Nkeubou Awougang
DATE           : 21/01/2022
DESCRIPTION    : Programme pour faire fonctionner un feu de signalisation suivant 
                  4 états (Clignotement_Vert, Vert, Jaune et Rouge) en utilisant les classes.
VERSION        : 0.0.1

*/

#ifndef TIMER_H
    #define TIMER_H // En même temps, on inclu Arduino.h

class Timer
{
    private:
        unsigned long timerStart  = 0;
        unsigned long timerTarget = 0;

    public:
        Timer(){}

        void startTimer(int msec);

        bool isTimerReady();
};

class LedFromColor
{
    private:
        short int BrocheColor;
        short int LedPIN;

    public:
        LedFromColor(){}

        void Initialisation();

        void Configure(short int PIN, short int Broche);

        short int GetPINLed();
};
#endif