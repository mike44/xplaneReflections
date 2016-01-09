#define GL_GLEXT_PROTOTYPES
#define TRUE 1
#define FALSE 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "XPLMDisplay.h"
#include "XPLMDataAccess.h"
#include "XPLMGraphics.h"
#include "XPLMPlugin.h"
#include <SDL/SDL.h>


#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#include "untitled.h"
#include "func.h"


XPLMDataRef		psi, theta, phi, locX, locY, locZ, camX, camY, camZ;

/* Set up some booleans */
#define TRUE  1
#define FALSE 0

/* Number of textures to load */
#define NUM_TEXTURES 2
XPLMTextureID gTexture[NUM_TEXTURES];

/* Status indicator */
int Statref = FALSE;

GLUquadricObj *quadratic;     /* Storage For Our Quadratic Objects */
GLuint	texture[6];			// Storage For 6 Textures (MODIFIED)



int	MyDrawCallback(
                                   XPLMDrawingPhase     inPhase,    
                                   int                  inIsBefore,    
                                   void *               inRefcon);

/* function to load in bitmap as a GL texture */
int LoadREFTextures( )
{

    int loop;

    /* Create storage space for the texture */
    SDL_Surface *TextureImage[2];

//    char filepth[500], filepath[500];
//    XPLMGetPluginInfo( XPLMGetMyID(), NULL,filepth, NULL,NULL);
    /* Load The Bitmap, Check For Errors, If Bitmap's Not Found Quit */
    if ( ( TextureImage[0] = SDL_LoadBMP( "data/bg.bmp" ) ) &&
         ( TextureImage[1] = SDL_LoadBMP( "data/reflect.bmp" ) ) )
    {

        /* Set the status to true */
        Statref = TRUE;

        /* Create The Texture */
      //  glGenTextures( NUM_TEXTURES, &texture[0] );
		XPLMGenerateTextureNumbers(&gTexture[NUM_TEXTURES], 1);
        for( loop = 0; loop <= 1; loop++ )
        {
            /* Create Nearest Filtered Texture */
        	XPLMBindTexture2d(gTexture[loop], 0);
        //    glBindTexture( GL_TEXTURE_2D, texture[loop] ); /* Gen Tex 0 And 1 */
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_NEAREST );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_NEAREST );
            glTexImage2D( GL_TEXTURE_2D, 0, 3, TextureImage[loop]->w, TextureImage[loop]->h,
                0, GL_BGR, GL_UNSIGNED_BYTE, TextureImage[loop]->pixels );

            /* Create Linear Filtered Texture */
        	XPLMBindTexture2d(gTexture[loop+2], 0);
        //    glBindTexture( GL_TEXTURE_2D, texture[loop+2] ); /* Gen Tex 2 And 3 */
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
            glTexImage2D( GL_TEXTURE_2D, 0, 3, TextureImage[loop]->w, TextureImage[loop]->h,
                0, GL_BGR, GL_UNSIGNED_BYTE, TextureImage[loop]->pixels );

            /* Create MipMapped Texture */
        	XPLMBindTexture2d(gTexture[loop+4], 0);
        //    glBindTexture( GL_TEXTURE_2D, texture[loop+4] ); /* Gen Tex 4 and 5 */
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
            gluBuild2DMipmaps( GL_TEXTURE_2D, 3, TextureImage[loop]->w, TextureImage[loop]->h,
                GL_BGR, GL_UNSIGNED_BYTE, TextureImage[loop]->pixels );
        }

    /* Free up any memory we may have used */
    for( loop = 0; loop <= 1; loop++ )
    {
        if ( TextureImage[loop] )
            SDL_FreeSurface( TextureImage[loop] );
    }
    }
    return Statref;
}


PLUGIN_API int XPluginStart(
						char *		outName,
						char *		outSig,
						char *		outDesc)
{
	/* First record our plugin information. */
	strcpy(outName, "DrawingHook");
	strcpy(outSig, "xplanesdk.examples.drawinghook");
	strcpy(outDesc, "A plugin that draws at a low level.");
	
	/* Next register teh drawing callback.  We want to be drawn 
	 * after X-Plane draws its 3-d objects. */
	XPLMRegisterDrawCallback(
					MyDrawCallback,	
					xplm_Phase_Objects, 	/* Draw when sim is doing objects */
					0,						/* After objects */
					NULL);					/* No refcon needed */
					
	/* Also look up our data refs. */
  	locX = XPLMFindDataRef("sim/flightmodel/position/local_x");
  	locY = XPLMFindDataRef("sim/flightmodel/position/local_y");
  	locZ = XPLMFindDataRef("sim/flightmodel/position/local_z");
  	camX = XPLMFindDataRef("sim/graphics/view/view_x");
  	camY = XPLMFindDataRef("sim/graphics/view/view_y");
  	camZ = XPLMFindDataRef("sim/graphics/view/view_z");
	phi = XPLMFindDataRef("sim/flightmodel/position/phi");//Roll
	theta = XPLMFindDataRef("sim/flightmodel/position/theta");//Pitch
	psi = XPLMFindDataRef("sim/flightmodel/position/psi");//Heading


	return 1;
}

PLUGIN_API void	XPluginStop(void)
{
	/* Unregitser the callback on quit. */
	XPLMUnregisterDrawCallback(
					MyDrawCallback,
					xplm_Phase_LastCockpit, 
					0,
					NULL);	

    /* Clean up our quadratic */
    gluDeleteQuadric( quadratic );

    /* Clean up our textures */
    glDeleteTextures( NUM_TEXTURES, &texture[0] );

}

PLUGIN_API void XPluginDisable(void)
{
}

PLUGIN_API int XPluginEnable(void)
{
	return 1;
}

PLUGIN_API void XPluginReceiveMessage(
					XPLMPluginID	inFromWho,
					long			inMessage,
					void *			inParam)
{
}



int	MyDrawCallback(
                                   XPLMDrawingPhase     inPhase,    
                                   int                  inIsBefore,    
                                   void *               inRefcon)
{

	glEnable(GL_TEXTURE_GEN_S);							// Enable Texture Coord Generation For S (NEW)
	glEnable(GL_TEXTURE_GEN_T);							// Enable Texture Coord Generation For T (NEW)

	//glBindTexture(GL_TEXTURE_2D, texture[1]); // This Will Select The Sphere Map
	XPLMBindTexture2d(texture[1], 0);
//	glPushMatrix();
	glTranslatef(XPLMGetDatad(locX), XPLMGetDatad(locY), XPLMGetDatad(locZ));
	glRotatef(XPLMGetDataf(psi), 0, -1, 0);//heading
	glRotatef(XPLMGetDataf(theta), 1, 0, 0);//pitch
	glRotatef(XPLMGetDataf(phi), 0, 0, -1);//roll

    /* Load in the texture */
    if (Statref == FALSE){
    	LoadREFTextures( );
    }
//    if ( !LoadGLTextures( ) )
//	return FALSE;


	glEnable(GL_TEXTURE_2D);							// Enable Texture Mapping
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	//glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	//glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

	  glEnable(GL_DEPTH_TEST);
	  glDepthFunc(GL_LEQUAL);
	//  glFrontFace(GL_CCW);
////
	  glEnable(GL_CULL_FACE);
//
	  glEnable(GL_NORMALIZE);

	  glShadeModel(GL_SMOOTH);

	  glEnable(GL_BLEND);
	  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//  glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_ALPHA);
	 // glDisable(GL_DEPTH_TEST);
	  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	quadratic=gluNewQuadric();							// Create A Pointer To The Quadric Object (Return 0 If No Memory)
	gluQuadricNormals(quadratic, GLU_SMOOTH);			// Create Smooth Normals
	gluQuadricTexture(quadratic, GL_TRUE);				// Create Texture Coords

	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP); // Set The Texture Generation Mode For S To Sphere Mapping (NEW)
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP); // Set The Texture Generation Mode For T To Sphere Mapping (NEW)

    InitMesh();

	    glRotatef(90, -1, 0, 0);
	 //   glRotatef(180, 0, -1, 0);
	    glRotatef(180, 0, 0, -1);
	    glScalef(-1,-1,1);
	    glTranslatef(0.0f,4.57f,-0.3f);

	  DrawAllMeshes();
		//glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glTranslatef(-XPLMGetDatad(locX), -XPLMGetDatad(locY), -XPLMGetDatad(locZ));

//	glPopMatrix();
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);

		
	return 1;
}
