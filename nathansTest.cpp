#include "nathansTest.h"

using std::cout;
using std::cerr;
using std::endl;


int main(int argc, char *argv[])
{
	glutInit(&argc, argv);

    glutInitDisplayMode (GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize (1024, 768);
    glutCreateWindow ("Quake 2's MD2 Model Viewer");
/*
    if (argc < 2)
    {
      cerr << "usage: " << argv[0] << " <path>" << endl;
      return -1;
    }
*/
    GLenum err = glewInit ();
    if (GLEW_OK != err)
    {
      // Problem: glewInit failed, something is seriously wrong.
      cerr << "Error: " << glewGetErrorString (err) << endl;
      shutdownApp ();
    }


    //initialize the test bullet
    //testBullet = new Bullet(2);
    // Initialize application
    atexit (shutdownApp);
	init();

    arVideoCapStart();
    argMainLoop( NULL, keyEvent, mainLoop );
	return (0);
}

static void   keyEvent( unsigned char key, int x, int y)
{
    /* quit if the ESC key is pressed */
    if( key == 0x1b ) {
        printf("*** %f (frame/sec)\n", (double)count/arUtilTimer());
        cleanup();
        exit(0);
    }

    //reset the game if the reset key is pressed
    if(key == 'r' || key == 'R')
        reset();
}

void reset(){
    init_enemies();
    bullet_vector.clear();  
}

//animation logic of the game
static void animLogic ()
{
  // Calculate frame per seconds
  static double current_time = 0;
  static double last_time = 0;
  static int n = 0;
  
  n++;
  current_time = timer.current_time;

  if( (current_time - last_time) >= 1.0 )
    {
      fps = n;
      n = 0;
      last_time = current_time;
    }

  if(!enemy1.isAlive){
    if(deathFrames > 0)
        deathFrames--;
    else
        bAnimated = false;
  }

  double dt = timer.current_time - timer.last_time;
  // Animate player
  if (bAnimated)
    {
      player->animate (frameRate * dt);
    }
  if(tAnimated){
      turret->animate (frameRate * dt);
  }
}

static void updateTimer (struct glut_timer_t *t)
{
  t->last_time = t->current_time;
  t->current_time = glutGet (GLUT_ELAPSED_TIME) * 0.001f;
}

/* main loop */
static void mainLoop(void)
{
    
    updateTimer (&timer);

    if (bAnimated || tAnimated)
        glutPostRedisplay ();

    animLogic();

    ARUint8         *dataPtr;
    ARMarkerInfo    *marker_info;
    int             marker_num;
    int             i, j, k;

    /* grab a vide frame */
    if( (dataPtr = (ARUint8 *)arVideoGetImage()) == NULL ) {
        arUtilSleep(2);
        return;
    }
    if( count == 0 ) arUtilTimerReset();
    count++;

    argDrawMode2D();

    argDispImage( dataPtr, 0,0 );

	glColor3f( 1.0, 0.0, 0.0 );
	glLineWidth(6.0);

    /* detect the markers in the video frame */
    if( arDetectMarker(dataPtr, thresh, &marker_info, &marker_num) < 0 ) {
        cleanup();
        exit(0);
    }

	for( i = 0; i < marker_num; i++ ) {
		argDrawSquare(marker_info[i].vertex,0,0);
	}

	/* check for known patterns */
    for( i = 0; i < objectnum; i++ ) {
		k = -1;
		for( j = 0; j < marker_num; j++ ) {
	        if( object[i].id == marker_info[j].id) {

				/* you've found a pattern */
				//printf("Found pattern: %d ",patt_id);
				glColor3f( 0.0, 1.0, 0.0 );
				argDrawSquare(marker_info[j].vertex,0,0);

				if( k == -1 ) k = j;
		        else /* make sure you have the best pattern (highest confidence factor) */
					if( marker_info[k].cf < marker_info[j].cf ) k = j;
			}
		}
		if( k == -1 ) {
			object[i].visible = 0;
			continue;
		}
		
		/* calculate the transform for each marker */
		if( object[i].visible == 0 ) {
            arGetTransMat(&marker_info[k],
                          object[i].marker_center, object[i].marker_width,
                          object[i].trans);
        }
        else {
            arGetTransMatCont(&marker_info[k], object[i].trans,
                          object[i].marker_center, object[i].marker_width,
                          object[i].trans);
        }
        object[i].visible = 1;
	}

        if(enemy1.health <= 0){
            enemy1.isAlive = false;
            player->setAnim("death");
        }

//        cout << "Printing enemy position!" << endl;
        
//       cout << "X: " << enemy1.x << " Y: " << enemy1.y << " Z: " << enemy1.z << endl;
        //Calculate the distance between the two objects, if both visible	
            if(object[0].visible == 1 && object[1].visible == 1){
                if(enemy1.isAlive){
                    move(object[0], object[1], &enemy1, marker_info, marker_num, object);
                }
            }

   /* 
    int curr_bullet; 

    for(curr_bullet = 0; curr_bullet < BULLET_NUM; curr_bullet++){
        if(bullets[curr_bullet].x > 500){
            bullets[curr_bullet].x = 0;
            bullets[curr_bullet].isAlive = true;
        }
        bullets[curr_bullet].x += bullets[curr_bullet].speed;
    }
    */
   
    for(int i = 0; i < bullet_vector.size(); i++){
        bullet_vector[i]->setX(bullet_vector[i]->getX() 
            + bullet_vector[i]->getXSpeed());

        //delete the bullet if it goes too far
        if(bullet_vector[i]->getX() > 500){
            bullet_vector.erase(bullet_vector.begin() + i);
        }
    }
    //testBullet->setX(testBullet->getX() + testBullet->getXSpeed());

    arVideoCapNext();

    global_marker_info = marker_info;
    global_marker_num = marker_num;

    draw( object, objectnum, marker_info, marker_num );

/*
    if (bAnimated)
        glutPostRedisplay ();
*/

    argSwapBuffers();

}

static void shutdownApp ()
{
  delete player;
  delete turret;
  delete teleport;
  delete roof;

  //Texture2DManager::kill ();
}

static void init( /*const string &path*/ )
{
  //
  // GLEW Initialization
  //
  
  
    ARParam  wparam;
	
    /* open the video path */
    if( arVideoOpen( vconf ) < 0 ) exit(0);
    /* find the size of the window */
    if( arVideoInqSize(&xsize, &ysize) < 0 ) exit(0);
    printf("Image size (x,y) = (%d,%d)\n", xsize, ysize);

    /* set the initial camera parameters */
    if( arParamLoad(cparam_name, 1, &wparam) < 0 ) {
        printf("Camera parameter load error !!\n");
        exit(0);
    }
    arParamChangeSize( &wparam, xsize, ysize, &cparam );
    arInitCparam( &cparam );
    printf("*** Camera Parameter ***\n");
    arParamDisp( &cparam );
/*
    if( (patt_id=arLoadPatt(patt_name)) < 0 ) {
        printf("pattern load error !!\n");
        exit(0);
    }
*/

    GLenum err = glewInit ();
    if (GLEW_OK != err)
    {
      // Problem: glewInit failed, something is seriously wrong.
      cerr << "Error: " << glewGetErrorString (err) << endl;
      shutdownApp ();
    }

    if( (object=read_ObjData(model_name, &objectnum)) == NULL ) exit(0);
    printf("Objectfile num = %d\n", objectnum);

    /* open the graphics window */
    argInit( &cparam, 1.0, 0, 0, 0, 0 );

    // Initialize timer
    timer.current_time = 0;
    timer.last_time = 0;

    //Init. camera input
    rot.x = 0.0f;   eye.x = 0.0f;
    rot.y = 0.0f;   eye.y = 0.0f;
    rot.z = 0.0f;   eye.z = 8.0f;

    //remove the trailing slash in the path name
//    string dirname (path);
//    if (dirname.find_last_of ('/') == dirname.length () - 1)
//    dirname.assign (dirname, 0, dirname.find_last_of ('/'));
    string dirname = "hueteotl"; 
    string dirname2 = "turret";
    string dirname3 = "gazebo";
    string dirname4 = "roof";

    // Get base dir for player if a *.md2 file is given
    if (dirname.find (".md2") == dirname.length () - 4)
    #ifdef _WIN32
    dirname.assign (dirname, 0, dirname.find_last_of ('\\'));
    #else
    dirname.assign (dirname, 0, dirname.find_last_of ('/'));
    #endif

    // Get base dir for player if a *.md2 file is given
    if (dirname2.find (".md2") == dirname2.length () - 4)
    #ifdef _WIN32
    dirname.assign (dirname2, 0, dirname2.find_last_of ('\\'));
    #else
    dirname.assign (dirname2, 0, dirname2.find_last_of ('/'));
    #endif

    // Get base dir for player if a *.md2 file is given
    if (dirname3.find (".md2") == dirname3.length () - 4)
    #ifdef _WIN32
    dirname.assign (dirname3, 0, dirname3.find_last_of ('\\'));
    #else
    dirname.assign (dirname3, 0, dirname3.find_last_of ('/'));
    #endif

    // Get base dir for player if a *.md2 file is given
    if (dirname4.find (".md2") == dirname3.length () - 4)
    #ifdef _WIN32
    dirname.assign (dirname3, 0, dirname3.find_last_of ('\\'));
    #else
    dirname.assign (dirname3, 0, dirname3.find_last_of ('/'));
    #endif

    // Load MD2 models
   
    try
    {
      player = new Md2Player (dirname);
      player->setScale (1.0f);
      player->setAnim ("run");
    }
    catch (std::runtime_error &err)
    {
      cerr << "Error: failed to load player from " << dirname << endl;
      cerr << "Reason: " << err.what () << endl;

      if (errno)
     cerr << strerror (errno) << endl;

      exit (-1);
    }

    try
    {
      turret = new Md2Player (dirname2);
      turret->setScale (1.0f);
      turret->setAnim ("pow");
    }
    catch (std::runtime_error &err)
    {
      cerr << "Error: failed to load player from " << dirname << endl;
      cerr << "Reason: " << err.what () << endl;

      if (errno)
     cerr << strerror (errno) << endl;

      exit (-1);
    }

    try
    {
      teleport = new Md2Player (dirname3);
      teleport->setScale (0.5f);
      teleport->setAnim ("baseframe");
    }
    catch (std::runtime_error &err)
    {
      cerr << "Error: failed to load player from " << dirname3 << endl;
      cerr << "Reason: " << err.what () << endl;

      if (errno)
     cerr << strerror (errno) << endl;

      exit (-1);
    }
    
    try
    {
      roof = new Md2Player (dirname4);
      roof->setScale (0.5f);
      roof->setAnim ("baseframe");
    }
    catch (std::runtime_error &err)
    {
      cerr << "Error: failed to load player from " << dirname4 << endl;
      cerr << "Reason: " << err.what () << endl;

      if (errno)
     cerr << strerror (errno) << endl;

      exit (-1);
    }

    //initalize enemies
    init_enemies();
    //initialize bullets
    init_bullets();

    //initialize openGL

/*    glClearColor (0.5f, 0.5f, 0.5f, 1.0f);
    glShadeModel (GL_SMOOTH);

    glEnable (GL_DEPTH_TEST);
    glEnable (GL_TEXTURE_2D);
    glEnable (GL_CULL_FACE);
    glEnable (GL_LIGHTING);
    glEnable (GL_LIGHT0);

    glCullFace (GL_BACK);
*/

    glutTimerFunc(BULLET_SPAWN_INTERVAL, spawnBullets, 0);
    
}

void init_enemies(void)
{
    enemy1.x = 0;
    enemy1.y = 0;
    enemy1.z = 0;
    enemy1.angle = 0;
    //enemy is the sphere
    enemy1.object_num = 0;
    enemy1.speed = 25;
    enemy1.health = 100;
    enemy1.isAlive = true;
    player->setAnim("run");
    bAnimated = true; 
    deathFrames = DEATH_FRAME_NUM; 
}

void init_bullets(void)
{
    int i;
    for(i = 0; i < BULLET_NUM; i++){
        bullets[i].x = -50*i;
        bullets[i].y = 0;
        bullets[i].z = 0;
        bullets[i].angle = 0;
        //enemy is the sphere
        bullets[i].object_num = 3;
        bullets[i].speed = 6;
        bullets[i].isAlive = true;
    }
}

/* cleanup function called when program exits */
static void cleanup(void)
{
    arVideoCapStop();
    arVideoClose();
    argCleanup();
}

/*
static void drawMD2(){

  // Clear window
  glClear (GL_DEPTH_BUFFER_BIT);
  
  glMatrixMode(GL_MODELVIEW);

  glLoadIdentity ();

    // Perform camera transformations
    glTranslated (-eye.x, -eye.y, -eye.z);
    glRotated (rot.x, 1.0f, 0.0f, 0.0f);
    glRotated (rot.y, 0.0f, 1.0f, 0.0f);
    glRotated (rot.z, 0.0f, 0.0f, 1.0f);

  glEnable (GL_DEPTH_TEST);

  if (bLightGL)
    glEnable (GL_LIGHTING);

  if (bTextured)
    glEnable (GL_TEXTURE_2D);

  // Draw objects
  //player->drawPlayerItp (bAnimated,
  //    static_cast<Md2Object::Md2RenderMode>(renderMode));

//  player->drawPlayerFrame (10, static_cast<Md2Object::Md2RenderMode>(renderMode));

	glutSolidTeapot(60);

  glDisable (GL_LIGHTING);
  glDisable (GL_TEXTURE_2D);
  glDisable (GL_DEPTH_TEST);

}
*/

static int  draw_bullet(double gl_para[16])
{
    
    GLfloat   mat_ambient[]				= {0.0, 0.0, 1.0, 1.0};
	GLfloat   mat_ambient_collide[]     = {1.0, 0.0, 0.0, 1.0};
    GLfloat   mat_flash[]				= {0.0, 0.0, 1.0, 1.0};
	GLfloat   mat_flash_collide[]       = {1.0, 0.0, 0.0, 1.0};
    GLfloat   mat_flash_shiny[] = {50.0};
    GLfloat   light_position[]  = {100.0,-200.0,200.0,0.0};
    GLfloat   ambi[]            = {0.1, 0.1, 0.1, 0.1};
    GLfloat   lightZeroColor[]  = {0.9, 0.9, 0.9, 0.1};
 
    argDrawMode3D();
    argDraw3dCamera( 0, 0 );
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd( gl_para );
 	// set the material 
    //glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambi);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);
    

    glMaterialfv(GL_FRONT, GL_SHININESS, mat_flash_shiny);	

    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);


 //   printf("Enemy coords %f, %f, %f\n", enemy1.x, enemy1.y, enemy1.z);
    int i;
    for(int i = 0; i < BULLET_NUM; i++){
        if(bullets[i].isAlive){
            if(bullets[i].x >= 0){ 
                glTranslatef( 0.0, -bullets[i].x, 0.0); 
               
                float bulletX = -bullets[i].x*gl_para[4] + gl_para[12];
                float bulletY = -bullets[i].x*gl_para[5] + gl_para[13];
                float bulletZ = -bullets[i].x*gl_para[6] + gl_para[14];
                //printf("Bullet %i coords: %f, %f, %f\n", i, bulletX, 
                 //  bulletY, bulletZ); 
                
                //some sort of correction for the bullet placement
                bulletZ += 30;
                
                float range = getRange(bulletX, bulletY, 0, 
                    enemy1.x, enemy1.y, 0);
                
                if(i == 0){
                   // printf("Diff x: %f .y: %f .z: %f\n", bulletX-enemy1.x, bulletY-enemy1.y, bulletZ-enemy1.z);
               //     printf("Bullet X: %f Y: %f Z: %f\n", bulletX, bulletY, bulletZ); 
                    //printf("Enemy X: %f Y: %f Z: %f\n", enemy1.x, enemy1.y, enemy1.z);
                }
                
                if( range <= 20 && enemy1.health >= -99){
                        enemy1.health -= 10;
                        bullets[i].isAlive = false;
                        //printf("Collision! Health: %d\n", enemy1.health);
                    }
                
                //don't draw if not alive
                    glutSolidSphere(3,12,6);

                glTranslatef( 0.0, bullets[i].x, 0.0);
                }
        }
    } 
    argDrawMode2D();

    return 0;
}

int drawBulletObj()
{
    
    for(int i = 0; i < bullet_vector.size(); i++){
        double* gl_para = new double[16];
        for(int j = 0; j < 16; j++)
            gl_para[j] = bullet_vector[i]->getTrans()[j];

        GLfloat   mat_ambient[]				= {0.0, 0.0, 1.0, 1.0};
        GLfloat   mat_ambient_collide[]     = {1.0, 0.0, 0.0, 1.0};
        GLfloat   mat_flash[]				= {0.0, 0.0, 1.0, 1.0};
        GLfloat   mat_flash_collide[]       = {1.0, 0.0, 0.0, 1.0};
        GLfloat   mat_flash_shiny[] = {50.0};
        GLfloat   light_position[]  = {100.0,-200.0,200.0,0.0};
        GLfloat   ambi[]            = {0.1, 0.1, 0.1, 0.1};
        GLfloat   lightZeroColor[]  = {0.9, 0.9, 0.9, 0.1};
     
        argDrawMode3D();
        argDraw3dCamera( 0, 0 );
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixd( gl_para );
        // set the material 
        //glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambi);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);
        

        glMaterialfv(GL_FRONT, GL_SHININESS, mat_flash_shiny);	

        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash);
        glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);

        //check for collisions
        float bulletX = -bullet_vector[i]->getX()*gl_para[4] + gl_para[12];
        float bulletY = -bullet_vector[i]->getX()*gl_para[5] + gl_para[13];
        float bulletZ = -bullet_vector[i]->getX()*gl_para[6] + gl_para[14];

       float range = getRange(bulletX, bulletY, 0, enemy1.x, enemy1.y, 0);

                
        if( range <= 20 && enemy1.health >= -99){
            enemy1.health -= 10;

            //erase the bullet, if it hits.
            bullet_vector.erase(bullet_vector.begin() + i);
        }

        glTranslatef( 0.0, -(bullet_vector[i]->getX()), 0.0); 

        glutSolidSphere(3,12,6);

        argDrawMode2D();

        delete gl_para;
    }
    return 0;
}

static int  draw_enemy(double gl_para[16])
{
    
    GLfloat   mat_ambient[]				= {0.0, 0.0, 1.0, 1.0};
	GLfloat   mat_ambient_collide[]     = {1.0, 0.0, 0.0, 1.0};
    GLfloat   mat_flash[]				= {0.0, 0.0, 1.0, 1.0};
	GLfloat   mat_flash_collide[]       = {1.0, 0.0, 0.0, 1.0};
    GLfloat   mat_flash_shiny[] = {50.0};
    GLfloat   light_position[]  = {100.0,-200.0,200.0,0.0};
    GLfloat   ambi[]            = {0.1, 0.1, 0.1, 0.1};
    GLfloat   lightZeroColor[]  = {0.9, 0.9, 0.9, 0.1};
 
    argDrawMode3D();
    argDraw3dCamera( 0, 0 );
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd( gl_para );
 	// set the material 
    //glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambi);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);
    


/*    glMaterialfv(GL_FRONT, GL_SHININESS, mat_flash_shiny);	

    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
*/

    /* draw a little teapot, short and stout */
    glRotatef( 90.0, 1.0, 0.0, 0.0 );
    glRotatef( enemy1.angle, 0.0, 1.0, 0.0);
 // player->drawPlayerFrame (10, static_cast<Md2Object::Md2RenderMode>(renderMode));

  player->drawPlayerItp (bAnimated, static_cast<Md2Object::Md2RenderMode>(renderMode));
    argDrawMode2D();

    return 0;
}

static int draw( ObjectData_T *object, int objectnum, ARMarkerInfo* marker_info, int marker_num )
{

    /*
    std::stringstream msg;  
    int enemyHealth = enemy1.health;
    msg << "Enemy health: " << enemyHealth;
    const char* newMsgStr = msg.str().c_str(); 
    */
    //const char* newMsgStr = "Enemy health: ";
    char newMsgStr[18]; 
    sprintf(newMsgStr, "Enemy health %d", enemy1.health);
    const unsigned char* newMsg = (const unsigned char*)newMsgStr; 
   // printf(newMsg);
    //printf("\n");
    glRasterPos2i(600, 1000);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, newMsg);

    //draw the MD2 model
// drawMD2();
 
    int     i;
    double  gl_para[16];
       
	glClearDepth( 1.0 );
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_LIGHTING);


//    // calculate the viewing parameters - gl_para 
//    for( i = 0; i < objectnum; i++ ) {
//        if( object[i].visible == 0 ) continue;
//       
//        if( object[i].id == 0 ){
//                object[i].trans[0][3] = enemy1.x;          
//                object[i].trans[1][3] = enemy1.y;
//                object[i].trans[2][3] = enemy1.z;  
//        } 
//        //printf(" X: %3.2f, Y: %3.2f, Z: %3.2f\n",enemy1.x, enemy1.y, enemy1.z);
//        argConvGlpara(object[i].trans, gl_para);
//        draw_object( object[i].id, gl_para);
//    }


    int l;
    //printf("Marker num: %d\n", marker_num); 
    for(l = 0; l < marker_num; l++){
        if(marker_info[l].cf > 0.5){
            int curr_id = marker_info[l].id;
            //ObjectData_T curr_obj;
            //curr_obj.id = curr_id;
            //curr_obj.visible = 1;
            double* foo = new double[2];
	    double bar;
	    double baz[3][4];
	    //arGetTransMat(&marker_info[l], foo, 
		//bar, baz);
	    arGetTransMat(&marker_info[l], object[curr_id].marker_center, 
		object[curr_id].marker_width, object[curr_id].trans);
            if(curr_id == enemy1.object_num){
                object[curr_id].trans[0][3] = enemy1.x;          
                object[curr_id].trans[1][3] = enemy1.y;
                object[curr_id].trans[2][3] = enemy1.z;   
            }       
            //test code, just put a bullet in front of the turret itself,
            //to see if things are actually working...
            /*
            else if(curr_id == 3){
                double newTrans[3][4];
                copyTrans(newTrans, object[curr_id].trans);          
                testBullet->setTrans(newTrans);
            }
            */
            argConvGlpara(object[curr_id].trans, gl_para);

            if(curr_id == 0) 
                draw_enemy(gl_para);
            else if(curr_id == 3){
             //   draw_bullet(gl_para);
                //printf("Drawing marker number %d\n", marker_num);
                //testBullet->setTrans(gl_para);
                draw_object(object[curr_id].id, gl_para);
            }
            else
                draw_object(object[curr_id].id, gl_para);  
        }
    }

    drawBulletObj();
    glDisable( GL_LIGHTING );
    glDisable( GL_DEPTH_TEST );
	

    return(0);
}
/*
static int draw( ObjectData_T *object, int objectnum )
{
    int     i;
    double  gl_para[16];
       
	glClearDepth( 1.0 );
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_LIGHTING);

    // calculate the viewing parameters - gl_para 
    for( i = 0; i < objectnum; i++ ) {
        if( object[i].visible == 0 ) continue;
        argConvGlpara(object[i].trans, gl_para);
        draw_object( object[i].id, gl_para);
    }
     
	glDisable( GL_LIGHTING );
    glDisable( GL_DEPTH_TEST );
	
    return(0);
}
*/

/* draw the user object */
static int  draw_object( int obj_id, double gl_para[16])
{
    GLfloat   mat_ambient[]				= {0.0, 0.0, 1.0, 1.0};
	GLfloat   mat_ambient_collide[]     = {1.0, 0.0, 0.0, 1.0};
    GLfloat   mat_flash[]				= {0.0, 0.0, 1.0, 1.0};
	GLfloat   mat_flash_collide[]       = {1.0, 0.0, 0.0, 1.0};
    GLfloat   mat_flash_shiny[] = {50.0};
    GLfloat   light_position[]  = {100.0,-200.0,200.0,0.0};
    GLfloat   ambi[]            = {0.1, 0.1, 0.1, 0.1};
    GLfloat   lightZeroColor[]  = {0.9, 0.9, 0.9, 0.1};
 
    argDrawMode3D();
    argDraw3dCamera( 0, 0 );
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd( gl_para );
//	  set the material 

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambi);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);

    //glMaterialfv(GL_FRONT, GL_SHININESS, mat_flash_shiny);	

	if(obj_id == 0){
		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash_collide);
		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient_collide);
		/* draw a sphere */
		glTranslatef( 0.0, 0.0, 30.0 );
		//glutSolidSphere(30,12,6);
	}
	else if(obj_id == 1) {
		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash);
		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
		/* draw a cube */
        glRotatef( 90.0, 1.0, 0.0, 0.0 );
		//glTranslatef( 0, 0, 0 );
        teleport->drawPlayerItp (bAnimated, static_cast<Md2Object::Md2RenderMode>(renderMode));
        roof->drawPlayerItp (bAnimated, static_cast<Md2Object::Md2RenderMode>(renderMode));
		//glutSolidCube(60);
	}
	else if(obj_id == 2){
		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash);
		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
		/* draw a little teapot, short and stout */
		glTranslatef( 0.0, 0.0, 30.0 );
	//	glutSolidTeapot(60);
	}
    else{
		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash);
		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
		/* draw a little teapot, short and stout */
		glTranslatef( 0.0, 0.0, 0.0 );
        glRotatef( 90.0, 1.0, 0.0, 0.0 );
        turret->drawPlayerItp (bAnimated, static_cast<Md2Object::Md2RenderMode>(renderMode));
            argDrawMode2D();
    }

    argDrawMode2D();

    return 0;
}

void spawnBullets(int num){
    int i;
    //printf("Marker num: %d\n", global_marker_num); 
    for(i = 0; i < global_marker_num; i++){ 
        if(global_marker_info[i].cf > 0.5){
            int curr_id = global_marker_info[i].id;

            arGetTransMat(&global_marker_info[i], object[curr_id].marker_center, 
                object[curr_id].marker_width, object[curr_id].trans);

            if(curr_id == 3){ //is a turret
                double  gl_para[16]; 
                argConvGlpara(object[curr_id].trans, gl_para);
                Bullet* newBullet = new Bullet(gl_para, BULLET_SPEED);
                bullet_vector.push_back(newBullet); 
            }
        }
    }
    glutTimerFunc(BULLET_SPAWN_INTERVAL, spawnBullets, 0);
}

/*
static void draw( void )
{
    double    gl_para[16];
    GLfloat   mat_ambient[]     = {0.0, 1.0, 0.0, 1.0};
    GLfloat   mat_flash[]       = {0.0, 1.0, 0.0, 1.0};
    GLfloat   mat_flash_shiny[] = {50.0};
    GLfloat   light_position[]  = {100.0,-200.0,200.0,0.0};
    GLfloat   ambi[]            = {0.1, 0.1, 0.1, 0.1};
    GLfloat   lightZeroColor[]  = {0.9, 0.9, 0.9, 0.1};
    
    argDrawMode3D();
    argDraw3dCamera( 0, 0 );
    glClearDepth( 1.0 );
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
    // load the camera transformation matrix 
    argConvGlpara(patt_trans, gl_para);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd( gl_para );

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambi);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_flash_shiny);	
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMatrixMode(GL_MODELVIEW);
    glTranslatef( 50.0, 0.0, 25.0 );
    glutSolidCube(50.0);
    glDisable( GL_LIGHTING );

    glDisable( GL_DEPTH_TEST );
}
*/
