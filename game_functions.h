#ifndef GAME_FUNCTIONS_H
#define GAME_FUNCTIONS_H
/* This file contains important utility / helper
functions that are to be used for the game. These
include moving the enemies.
*/
#include "object.h"
#include "game_entity.h"
#include <AR/ar.h>

#define PROXIMITY_THRESH 12

/*Move curr_entity towards object1 from object2 
*/
void move_towards(double trans1[][4], double trans2[][4], GameEntity_T *curr_entity);

/*
Give the entity the object that correspond to the source, and the
object that corresponds to the destination, and the marker_info
array. This should figure out how to move the entity correctly.
*/

void move(ObjectData_T source, ObjectData_T dest, GameEntity_T *curr_entity, ARMarkerInfo *marker_info, int marker_num, ObjectData_T *object); 

//gets the range between two points 1 and 2 in 3D space
float getRange(float x1, float y1, float z1, float x2, float y2, float z2); 

/* Determine if the marker is an "eligible" marker on the path.
In other words, does the marker bring us any closer to the destination?
We must pass the transform of the current marker, and also the 
transform of the end, and the transform of the current entity

Also, the marker can't be too close to the current entity. If it is,
then the marker is "ineligible"

Returns 1 if eligible. 0 if ineligible.
*/
int isEligible(double currMarkerTrans[][4], GameEntity_T *curr_entity, 
double endTrans[][4], double startTrans[][4]);

//Copies a 3x4 translation matrix into another one
void copyTrans(double destTrans[][4], double srcTrans[][4]); 
#endif
