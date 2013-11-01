// Template for GP1, version 1
// IGAD/NHTV - Jacco Bikker - 2006-2013

#include "string.h"
#include "game.h"
#include "surface.h"
#include "stdlib.h"
#include "template.h"
#include "raytracer.h"
#include "Renderer.h"
#include <SDL.h>
#include <AntTweakBar.h>
#include <gl/GL.h>
#include <gl/GLU.h>

using namespace Tmpl8;

Tracer tracer;
Renderer renderer;

float3 g_CameraPosition(0,0,-4);
float g_CameraRotationY, g_CameraRotationX;
bool g_KeyState[512];


void Game::Init()
{
	memset(g_KeyState, 0, sizeof(bool)*512);
	
	tracer.init();
	renderer.init();
	renderer.setTracer(&tracer);

}
void Game::KeyUp(unsigned int keycode){
	g_KeyState[keycode]=false;
}
void Game::KeyDown(unsigned int keycode){
	g_KeyState[keycode]=true;
}
void Game::Tick( float a_DT )
{
	//Camera movement
	float3 movement(0,0,0);
	if(g_KeyState[SDL_SCANCODE_W]) movement.z+=1.0f;
	if(g_KeyState[SDL_SCANCODE_S]) movement.z-=1.0f;
	if(g_KeyState[SDL_SCANCODE_A]) movement.x-=1.0f;
	if(g_KeyState[SDL_SCANCODE_D]) movement.x+=1.0f;
	if(SQRLENGTH(movement) > EPSILON){
		tracer.clear();

		if(SQRLENGTH(movement) > 1) Normalize(movement);
		g_CameraPosition+=movement*a_DT*0.1f;
	}


	//Camera rotation
	float3 rotation(0,0,0);
	if(g_KeyState[SDL_SCANCODE_LEFT]) rotation.y-=1;
	if(g_KeyState[SDL_SCANCODE_RIGHT]) rotation.y+=1;
	if(g_KeyState[SDL_SCANCODE_UP]) rotation.x+=1;
	if(g_KeyState[SDL_SCANCODE_DOWN]) rotation.x-=1;
	if(SQRLENGTH(rotation) > EPSILON){
		tracer.clear();

		if(SQRLENGTH(rotation) > 1) Normalize(movement);
		g_CameraRotationX += rotation.x*a_DT*0.1f;
		g_CameraRotationY += rotation.y*a_DT*0.1f;
	}

	//save to file
	if(g_KeyState[SDLK_SPACE]){
		m_Surface->SaveImage("output.jpg");
	}

	const float cosz = cosf(0);
	const float sinz = sinf(0);
	const float cosx = cosf(g_CameraRotationX);
	const float sinx = sinf(g_CameraRotationX);
	const float cosy = cosf(g_CameraRotationY);
	const float siny = sinf(g_CameraRotationY);
	float3 forward;
	forward.x = cosx*siny;
	forward.y = sinx;
	forward.z = cosx*cosy;

	tracer.camera().Set(g_CameraPosition, forward);
	
}

void drawNode_r(BVHNode& node, int level=0){
	glColor3f(1, 1-level*0.1f, 1-level*0.1f);
	glBegin(GL_QUADS);
	//X planes
	glVertex3f(node.m_Bounds.min.x, node.m_Bounds.min.y, node.m_Bounds.min.z);
	glVertex3f(node.m_Bounds.min.x, node.m_Bounds.max.y, node.m_Bounds.min.z);
	glVertex3f(node.m_Bounds.min.x, node.m_Bounds.max.y, node.m_Bounds.max.z);
	glVertex3f(node.m_Bounds.min.x, node.m_Bounds.min.y, node.m_Bounds.max.z);
	glVertex3f(node.m_Bounds.max.x, node.m_Bounds.min.y, node.m_Bounds.min.z);
	glVertex3f(node.m_Bounds.max.x, node.m_Bounds.max.y, node.m_Bounds.min.z);
	glVertex3f(node.m_Bounds.max.x, node.m_Bounds.max.y, node.m_Bounds.max.z);
	glVertex3f(node.m_Bounds.max.x, node.m_Bounds.min.y, node.m_Bounds.max.z);
	//Y planes
	glVertex3f(node.m_Bounds.min.x, node.m_Bounds.min.y, node.m_Bounds.min.z);
	glVertex3f(node.m_Bounds.max.x, node.m_Bounds.min.y, node.m_Bounds.min.z);
	glVertex3f(node.m_Bounds.max.x, node.m_Bounds.min.y, node.m_Bounds.max.z);
	glVertex3f(node.m_Bounds.min.x, node.m_Bounds.min.y, node.m_Bounds.max.z);
	glVertex3f(node.m_Bounds.min.x, node.m_Bounds.max.y, node.m_Bounds.min.z);
	glVertex3f(node.m_Bounds.max.x, node.m_Bounds.max.y, node.m_Bounds.min.z);
	glVertex3f(node.m_Bounds.max.x, node.m_Bounds.max.y, node.m_Bounds.max.z);
	glVertex3f(node.m_Bounds.min.x, node.m_Bounds.max.y, node.m_Bounds.max.z);
	glEnd();

	if(node.m_Left){
		drawNode_r(*node.m_Left, level+1);
		drawNode_r(*node.m_Right, level+1);
	}
}
void Game::Draw(){
	renderer.render();
	
	/*glPushAttrib(GL_TRANSFORM_BIT|GL_POLYGON_BIT|GL_ENABLE_BIT|GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	const double persp = 2*atan(0.5)*180.0/PI;
	gluPerspective(persp, SCRWIDTH/(double)SCRHEIGHT, 0.01, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glScalef(1, 1, -1);//convert to right handed coordinate system
	glRotatef(-g_CameraRotationY*180/PI, 0, 1, 0);
	glTranslatef(-g_CameraPosition.x, -g_CameraPosition.y, -g_CameraPosition.z);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	drawNode_r(*tracer.scene.root);


	//Clean up after drawing
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glPopAttrib();*/
}