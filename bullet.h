#ifndef BULLET_H
#define BULLET_H
class Bullet{
private:
    //the current translation matrix of the bullet
    double currTrans[16];
    float currXSpeed; 
    //x position of the bullet, relative to its own coordinate frame
    float x; 
public:
    //start bullet with a given transformation,
    //and a speed for travel in the forward x-direction
    Bullet(double trans[16], float Xspeed);
    Bullet(float Xspeed);
    ~Bullet();
    //move the bullet forward in the x direction by one
    //void stepForward(); 
    //read the bullet's transform into a buffer
    double* getTrans(); 

    void setTrans(double newTrans[16]);
    
    void setX(float newX); 

    float getX();
    
    float getXSpeed();
    
};
#endif
