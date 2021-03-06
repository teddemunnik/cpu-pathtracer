// Template for GP1, version 1
// IGAD/NHTV - Jacco Bikker - 2006-2013
#pragma warning (disable : 4530) // complaint about exception handler

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

float3 g_CameraPosition(0,0,-2);
float3 g_CameraRotation(0,0,0);
float g_FocalDist = 5.0f;
float g_ApertureSize = 0.0f;
bool g_KeyState[512];
Tracer::ShadeMode g_ShadeMode = Tracer::kShadeComplex;

TwBar* g_Bar;

void Game::Init()
{
	memset(g_KeyState, 0, sizeof(bool)*512);
	
	tracer.init();
	renderer.init();
	renderer.setTracer(&tracer);

	g_Bar = TwNewBar("Camera Setting");
	TwAddVarRO(g_Bar, "Aperture size[+/-]", TW_TYPE_FLOAT, &g_ApertureSize, nullptr);
	TwAddVarRO(g_Bar, "Focal Distance[pageup/pagedown]", TW_TYPE_FLOAT, &g_FocalDist, nullptr);

}
void Game::KeyUp(unsigned int keycode){
	g_KeyState[keycode]=false;
}
void Game::KeyDown(unsigned int keycode){
	g_KeyState[keycode]=true;
}
bool wasTab = false;
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

		if(g_KeyState[SDL_SCANCODE_LSHIFT]) movement*=5.0f;

		//Extract rotation matrix
		float4x4 transform = tracer.camera().transform();
		transform.w = float4(0,0,0,1);
		transform.x.w = 0;
		transform.y.w = 0;
		transform.z.w = 0;

		g_CameraPosition+= (transform *  float4(movement*a_DT, 0.0f)).xyz;
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
		g_CameraRotation.x += rotation.x*a_DT;
		g_CameraRotation.y += rotation.y*a_DT;
	}


	//Focal distance
	float focalDist = 0.0f;
	if(g_KeyState[SDL_SCANCODE_PAGEUP]) focalDist += a_DT;
	if(g_KeyState[SDL_SCANCODE_PAGEDOWN]) focalDist -= a_DT;
	
	//Aperture size
	float deltaAperture = 0.0f;
	if(g_KeyState[SDL_SCANCODE_MINUS]) deltaAperture -= a_DT*0.05f;
	if(g_KeyState[SDL_SCANCODE_EQUALS]) deltaAperture += a_DT*0.05f;

	if(fabsf(focalDist) > 0.0001f || fabsf(deltaAperture) > 0.0001f){
		g_FocalDist += focalDist;
		g_ApertureSize += deltaAperture;
		tracer.camera().setAperture(g_ApertureSize, g_FocalDist);
		tracer.clear();
	}

	//Toggle simple shading
	if(g_KeyState[SDL_SCANCODE_TAB]) wasTab=true;
	else if(wasTab){
		g_ShadeMode = g_ShadeMode == Tracer::kShadeComplex ? Tracer::kShadeSimple : Tracer::kShadeComplex;
		tracer.setShadingMode(g_ShadeMode);
		wasTab = false;
		tracer.clear();
	}
	
	tracer.camera().set(g_CameraPosition, g_CameraRotation);
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
}
