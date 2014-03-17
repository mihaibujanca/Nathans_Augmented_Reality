#ifndef GAME_ENTITY_H
#define GAME_ENTITY_H

typedef struct {
    char       name[256];
    //position of the entity
    float x; 
    float y;
    float z;
    /*angle, in the y direction.
    this is where the enemy is facing*/
    float angle;   
    
    //Which ARToolkit object corresponds to this entity
    int object_num;
    
    int health;
    //speed at which the object moves
    float speed;
    
    bool isAlive;
} GameEntity_T;
#endif
