#ifndef NATHANSTEST_H
#define NATHANSTEST_H

#ifdef _WIN32
#  include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstdarg>
#include <cerrno>
#include <GL/glew.h>

#ifndef __APPLE__
#  include <GL/glut.h>
#else
#  include <GLUT/glut.h>
#endif
#include <GL/freeglut_ext.h>
#include <AR/gsub.h>
#include <AR/param.h>
#include <AR/ar.h>
#include <AR/video.h>

#include "bullet.h"
#include "object.h"
#include "game_entity.h"
#include "game_functions.h"

#include "md2loader/Md2Player.h"
#include "md2loader/TextureManager.h"

#define BULLET_NUM 0
#define BULLET_SPEED 10
#define DEATH_FRAME_NUM 8

//bullet spawn interval is in milliseconds
#define BULLET_SPAWN_INTERVAL 300

// Camera
struct Vector3d
{
  float x, y, z;

} rot, eye;

// Timer
struct glut_timer_t
{
  double current_time;
  double last_time;

} timer;

//model for the player, turret, and teleport.
//there may be multiple turrets, but they are all drawn the same way 
Md2Player *player;
Md2Player *turret;
Md2Player *teleport;
Md2Player *roof;

bool bTextured = true;
bool bLightGL = true;
bool bAnimated = true;
bool tAnimated = true;
int verbose = 2;
int renderMode = 0;
int frameRate = 7;
int fps = 0;
int deathFrames = DEATH_FRAME_NUM;
vector<string> skinList;
vector<string> animList;

#define COLLIDE_DIST 30000.0

#ifdef _WIN32
char			*vconf = "Data\\WDM_camera_flipV.xml";
#else
char			*vconf = "";

#endif

int             xsize, ysize;
int             thresh = 100;
int             count = 0;

char           *cparam_name    = "Data/camera_para.dat";
ARParam         cparam;

char           *patt_name      = "Data/patt.hiro";
int             patt_id;
double          patt_width     = 80.0;
double          patt_center[2] = {0.0, 0.0};
double          patt_trans[3][4];

char            *model_name = "Data/object_data3";
ObjectData_T    *object;
int             objectnum;
ARMarkerInfo*   global_marker_info;
int             global_marker_num = 0;

/*Put a collection of enemies / other game entities
here. For now, we have one enemy, a fearsome sphere*/
GameEntity_T enemy1;
//start out with t bullet
GameEntity_T bullets[BULLET_NUM];

int sphereTowards1 = 1;   
static void   init();
static void   cleanup(void);
static void   keyEvent( unsigned char key, int x, int y);
static void   mainLoop(void);
static int draw( ObjectData_T *object, int objectnum, ARMarkerInfo *marker_info, int marker_num);
static int  draw_object( int obj_id, double gl_para[16] );
static void   init_enemies(void);
static void init_bullets(void);
static void shutdownApp();

//resets the game when the correct key is pressed 
//resets enemy model back to original position
//clears array of bullets
//reset enemy health
void reset();

//spawns a bullet. This function tied to glutTimer()
void spawnBullets(int);

int drawBulletObj();

Bullet* testBullet;
vector<Bullet*> bullet_vector;

#endif
