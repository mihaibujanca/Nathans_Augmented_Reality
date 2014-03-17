#include "game_functions.h"
#include <math.h>
#include <stdlib.h>
#include <AR/gsub.h>
#define _USE_MATH_DEFINES
void move_towards(double trans1[][4], double trans2[][4], GameEntity_T *curr_entity)
{ 
                //position of the source
                float Xpos0 = trans1[0][3];          
                float Ypos0 = trans1[1][3];
                float Zpos0 = trans1[2][3];
                
                //position of the destination
                float Xpos1 = trans2[0][3];          
                float Ypos1 = trans2[1][3];
                float Zpos1 = trans2[2][3];
                
                /* dx, dy, dz can be thought of as the vector
                between the destination and the current entity
                */
                float dx = Xpos1 - curr_entity->x;
                float dy = Ypos1 - curr_entity->y;
                float dz = Zpos1 - curr_entity->z;
                //printf("dx: %f, dy: %f, dz: %f", dx, dy, dz);
               
                /* srcFwd can be thought of as the "forward"
                vector of the source. I.e. it is the direction
                the entity should face at the very beginning 
                 
                float srcFwdX = 0 
                float srcFwdY = -1;
                float srcFwdZ = 0 
                */

                double rangeSquared = dx*dx+dy*dy+dz*dz; 
                double range = sqrt(rangeSquared);

                float dxNorm = 0;
                float dyNorm = 0;
                float dzNorm = 0;

                if(dx < 0)
                        dxNorm = -sqrt(-dx/rangeSquared);
                else
                        dxNorm = sqrt(dx/rangeSquared);
                if(dy < 0)
                        dyNorm = -sqrt(-dy/rangeSquared);
                else
                        dyNorm = sqrt(dy/rangeSquared);
                if(dz < 0)
                        dzNorm = -sqrt(-dz/rangeSquared);
                else
                        dzNorm = sqrt(dz/rangeSquared);

                //convert the transform matrix of source marker into
                //standard OpenGL matrix format.

                        
                double srcGlTrans[16];
                argConvGlpara(trans1, srcGlTrans);
                
                //arrowTip refers to the point at (0, -1, 0)
                //in the coordinate system of the starting marker
                float arrowTip[3];

                //derived from multiplying the transform of 
                //the starting marker with (0, -1, 0)
                arrowTip[0] = -srcGlTrans[4] + srcGlTrans[12];
                arrowTip[1] = -srcGlTrans[5] + srcGlTrans[13];
                arrowTip[2] = -srcGlTrans[6] + srcGlTrans[14];
              
                //arrow vector. It's the tip minus the start
                //the start is the coordinates of the source marker. 
                float arrowVector[3] = { arrowTip[0] - Xpos0,
                    arrowTip[1] - Ypos0, arrowTip[2] - Zpos0 }; 

                float arrowLength = getRange(arrowTip[0], arrowTip[1],
                    arrowTip[2], Xpos0, Ypos0, Zpos0); 
                /*
                printf("Printing last row of srcGlTrans!\n");
                int i;
                for(i = 0; i < 16; i++){
                    if(i%4 == 0)
                        printf("\n");
                    printf("%f ", srcGlTrans[i]);
                }
                printf("\n");
                */

                //Compute the world coordinates of the point 
                //(0, -1, 0) in the starting marker's coordinate
                //system.
                 
                
                //cosine of angle the enemy is supposed to face
                //derived from the dot product of dx, dy, dz with
                //the source forward vector. (x and z of source 
                //forward vector is zero, its magnitude is one. 
                //the magnitude of dx, dy, dz is range)
                float cosFaceAngle = (dx*arrowVector[0]
                + dy*arrowVector[1] + dz*arrowVector[2]) / 
                (range*arrowLength);
                float faceAngle = acos(cosFaceAngle) * (180/M_PI);
               // printf(" Face angle: %f\n", faceAngle);
                
                //convert to degrees
                curr_entity->angle = faceAngle;

                curr_entity->x += curr_entity->speed*dxNorm;
                curr_entity->y += curr_entity->speed*dyNorm;
                curr_entity->z += curr_entity->speed*dzNorm;
}

void move(ObjectData_T source, ObjectData_T dest, GameEntity_T *curr_entity, ARMarkerInfo *marker_info, int marker_num, ObjectData_T *object){

    float sourceX = source.trans[0][3];          
    float sourceY = source.trans[1][3];
    float sourceZ = source.trans[2][3];

    float destX = dest.trans[0][3];          
    float destY = dest.trans[1][3];
    float destZ = dest.trans[2][3];

    //reset position back to the source if it's uninitialized
    if(curr_entity->x == 0 && curr_entity->y == 0 && curr_entity->z == 0){ 
        //printf(" XPos0: %3.2f, YPos0: %3.2f, ZPos0: %3.2f\n",Xpos0, Ypos0, Zpos0);
            curr_entity->x = sourceX;
            curr_entity->y = sourceY; 
            curr_entity->z = sourceZ; 
    }

    //if the entity is at the dest, reset the position back to the source 
    if(getRange(curr_entity->x, curr_entity->y, curr_entity->z, 
        destX, destY, destZ) < PROXIMITY_THRESH){

            curr_entity->x = sourceX;
            curr_entity->y = sourceY; 
            curr_entity->z = sourceZ; 
            curr_entity->health = 100;
    }  

    /* Instantiate an array of trans objects that is the same
    length as the marker_info list. Call this eligible_markers. */  
    ObjectData_T* eligible_markers[marker_num];
    
    //initialize this array to null
    int i;
    for(i = 0; i < marker_num; i++){
        eligible_markers[i] = NULL;
    } 
    /*Iterate through the marker_info list. Copy any marker 
    that is closer to destination than the entity to eligible_markers.*/ 
    int e_index = 0; //index of eligible_markers
    for(i = 0; i < marker_num; i++){ 
            int curr_id = marker_info[i].id;
            //only move towards it if it is the endpoint / a waypoint
            if(curr_id == 1 || curr_id == 2){ 
                arGetTransMat(&marker_info[i], object[curr_id].marker_center, 
                    object[curr_id].marker_width, object[curr_id].trans);

                if(isEligible(object[curr_id].trans, curr_entity, dest.trans, source.trans)){
                    //printf("Marker %d is eligible!\n", i); 
                    ObjectData_T* new_obj = (ObjectData_T*)malloc(sizeof(ObjectData_T));
                    copyTrans(new_obj->trans, object[curr_id].trans);
                    eligible_markers[e_index] = new_obj;
                    e_index++;
                }  

            }
            
    }
   /* iterate through eligible_markers and find the marker at 
      minimum distance from the current entity
    */
    
    ObjectData_T* first_obj = eligible_markers[0];
    if(first_obj != NULL){

        double min_marker_trans[3][4];
        copyTrans(min_marker_trans, first_obj->trans);
        float min_marker_dist = getRange(first_obj->trans[0][3], first_obj->trans[1][3], first_obj->trans[2][3], curr_entity->x, curr_entity->y, curr_entity->z); 

        for(i = 0; i < e_index; i++){
            ObjectData_T* curr_obj = eligible_markers[i];  
            float curr_obj_dist = getRange(curr_obj->trans[0][3],
                curr_obj->trans[1][3], curr_obj->trans[2][3], curr_entity->x,
                curr_entity->y, curr_entity->z); 
            
            //if we fond a smaller one
            if(curr_obj_dist < min_marker_dist){
                copyTrans(min_marker_trans, curr_obj->trans);
                min_marker_dist = curr_obj_dist; 
            }
        }    
    //call move_towards - move source towards the minimum marker    
    move_towards(source.trans, min_marker_trans, curr_entity);    
    
    }

    for(i = 0; i < e_index; i++){
        free(eligible_markers[e_index]);
    }

} 

int isEligible(double currMarkerTrans[][4], GameEntity_T *curr_entity, 
double endTrans[][4], double startTrans[][4]){

    float currMarkerX = currMarkerTrans[0][3];          
    float currMarkerY = currMarkerTrans[1][3];
    float currMarkerZ = currMarkerTrans[2][3];

    float currEntityX = curr_entity->x;          
    float currEntityY = curr_entity->y;
    float currEntityZ = curr_entity->z;

    float endX = endTrans[0][3];          
    float endY = endTrans[1][3];
    float endZ = endTrans[2][3];

    //subtract start from end to get vector ES
    float ES[3]; //ES[0] is x coord, [1] is y coord, [2] is z coord
    ES[0] = endX - startTrans[0][3];   
    ES[1] = endY - startTrans[1][3];   
    ES[2] = endZ - startTrans[2][3];   

    //subtract start from POSITION of entity to get vector PS
    float PS[3]; 
    PS[0] = currEntityX - startTrans[0][3];
    PS[1] = currEntityY - startTrans[1][3]; 
    PS[2] = currEntityZ - startTrans[2][3];

    //subtract start from current marker position to get vector MS
    float MS[3]; 
    MS[0] = currMarkerX - startTrans[0][3]; 
    MS[1] = currMarkerY - startTrans[1][3]; 
    MS[2] = currMarkerZ - startTrans[2][3]; 

    //magPS = length of PS, magMS = length of MS, magES = length of ES
    float magPS = getRange(0, 0, 0, PS[0], PS[1], PS[2]);
    float magMS = getRange(0, 0, 0, MS[0], MS[1], MS[2]);
    float magES = getRange(0, 0, 0, ES[0], ES[1], ES[2]);
    //cosP = cosine of POSITION angle = (PS dot ES) / (magPS*magES) 
    float cosP = (PS[0]*ES[0] + PS[1]*ES[1] + PS[2]*ES[2]) / (magPS*magES);
    //cosM = cosine of marker angle = (MS dot ES) / (magMS*magES) 
    float cosM = (MS[0]*ES[0] + MS[1]*ES[1] + MS[2]*ES[2]) / (magMS*magES);
    if(cosP < 0)
        cosP = -cosP;
    if(cosM < 0)
        cosM = -cosM;
    //magMSProj = magMS * cosM
    float magMSProj = magMS * cosM; 
    //magESProj = magES * cosE
    float magPSProj = magPS * cosP; 

    //printf("MagMSProj: %f, MagESProj: %f\n", magMSProj, magESProj);
    
    //if currMarkerTrans and currEntity trans are too close, reject
    if(getRange(currMarkerX, currMarkerY, currMarkerZ, 
        currEntityX, currEntityY, currEntityZ) < PROXIMITY_THRESH){
       return 0; 
    }
    //if magMSproj < magPSProj, reject.
    if(magMSProj < magPSProj){
        return 0;
    }
    /*
    //distance from marker to end
    float markerToEnd = getRange(currMarkerX, currMarkerY, currMarkerZ, 
        endX, endY, endZ);

    //distance from current entity location to end 
    float entityToEnd = getRange(currEntityX, currEntityY, currEntityZ, 
        endX, endY, endZ); 
    
    if(markerToEnd > entityToEnd){
        return 0;
    }
    */
    else{
        return 1;
    }
}

float getRange(float x1, float y1, float z1, float x2, float y2, float z2){
    
    float dx = 0;
    float dy = 0;
    float dz = 0;

    dx = x1 - x2; 
    dy = y1 - y2; 
    dz = z1 - z2; 

    double rangeSquared = dx*dx+dy*dy+dz*dz; 
    return sqrt(rangeSquared);
} 

void copyTrans(double destTrans[][4], double srcTrans[][4]){
    int row = 0;
    int col = 0;
    for(row = 0; row < 3; row++){
        for(col = 0; col < 4; col++){
            destTrans[row][col] = srcTrans[row][col];
        }
    } 
}
