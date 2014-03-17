#include "bullet.h"
#include "game_functions.h"

Bullet::Bullet(double trans[16], float Xspeed){
    x = 0;
    currXSpeed = Xspeed; 
    for(int i = 0; i < 16; i++)
        currTrans[i] = trans[i];
}

//initializes transform to zeroes
Bullet::Bullet(float Xspeed){ 
    x = 0;
    currXSpeed = Xspeed; 
    for(int i = 0; i < 16; i++)
        currTrans[i] = 0;
}

Bullet::~Bullet(){

}

//increment the value corresponding to the x value
//in the transform of the bullet
/*
void Bullet::stepForward(){
    currTrans[0][3] += currXSpeed;
}
*/

void Bullet::setTrans(double trans[16]){
    for(int i = 0; i < 16; i++)
        currTrans[i] = trans[i];
} 

double* Bullet::getTrans(){
    return currTrans; 
}


void Bullet::setX(float newX){ 
    x = newX;
}

float Bullet::getX(){
    return x;
}

float Bullet::getXSpeed(){
    return currXSpeed;
}
