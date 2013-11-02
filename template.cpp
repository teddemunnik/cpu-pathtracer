// Template for GP1, version 1
// IGAD/NHTV - Jacco Bikker - 2006-2013

// Note:
// This version of the template attempts to setup a rendering surface in system RAM
// and copies it to VRAM using DMA. On recent systems, this yields extreme performance,
// and flips are almost instant. For older systems, there is a fall-back path that
// uses a more conventional approach offered by SDL. If your system uses this, the
// window caption will indicate this. In this case, you may want to tweak the video
// mode setup code for optimal performance.

#pragma warning (disable : 4530) // complaint about exception handler
#pragma warning (disable : 4273)
#pragma warning (disable : 4311) // pointer truncation from HANDLE to long


#include <Windows.h>
#include <GL/glew.h>
#include <GL/wglew.h>
#include <io.h>
#include <ios>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <SDL.h>
#include <fcntl.h>
#include <anttweakbar.h>
#include <FreeImage.h>

#include "game.h"
#include "surface.h"
#include "template.h"


namespace Tmpl8 { 
void NotifyUser( char* s )
{
	HWND hApp = FindWindow( NULL, "Template" );
	MessageBox( hApp, s, "ERROR", MB_OK );
	exit( 0 );
}
}

using namespace Tmpl8;
using namespace std;


static int SCRPITCH = 0;
int ACTWIDTH, ACTHEIGHT;
static bool FULLSCREEN = false, firstframe = true;

Game* game = 0;
double lastftime = 0;
LARGE_INTEGER lasttime, ticksPS;

SDL_Window* g_Window;
SDL_GLContext g_Context;


bool init()
{
	if(glGetError()) return false;
	glViewport( 0, 0, SCRWIDTH, SCRHEIGHT );
	QueryPerformanceFrequency( &ticksPS );
	return true;
}

void redirectIO()
{
	static const WORD MAX_CONSOLE_LINES = 500;
	int hConHandle;
	long lStdHandle;
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE *fp;
	AllocConsole();
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),
	&coninfo);
	coninfo.dwSize.Y = MAX_CONSOLE_LINES;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE),
	coninfo.dwSize);
	lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stdout = *fp;
	setvbuf( stdout, NULL, _IONBF, 0 );
	lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "r" );
	*stdin = *fp;
	setvbuf( stdin, NULL, _IONBF, 0 );
	lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stderr = *fp;
	setvbuf( stderr, NULL, _IONBF, 0 );
	ios::sync_with_stdio();
}

void swap()
{
	game->Draw();

	//TwDraw();
	SDL_GL_SwapWindow(g_Window);
}

void FreeImage_DebugMessage(FREE_IMAGE_FORMAT format, const char* message){
	printf("FreeImage: %s\n", message);
}
void CALLBACK OpenGL_DebugMessage(unsigned int source, unsigned int type, unsigned int id, unsigned int severity, int length, const char* message, void* userParam){
	printf("OpenGL: %s\n", message);
}
int main( int argc, char **argv ) 
{  
	//Console window
	redirectIO();
	
	//FreeImage + Debug
	FreeImage_Initialise();
	FreeImage_SetOutputMessage(&FreeImage_DebugMessage);

	//SDL Window + GLEW +  OpenGL 3.2 + Debug
	//Switched to SDL2 since Nsight requires a opengl 3+ context
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG|SDL_GL_CONTEXT_DEBUG_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_Init( SDL_INIT_VIDEO );

	g_Window = SDL_CreateWindow("Tracer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCRWIDTH, SCRHEIGHT, SDL_WINDOW_SHOWN|SDL_WINDOW_OPENGL);
	g_Context = SDL_GL_CreateContext(g_Window);
	SDL_GL_SetSwapInterval(0);

	glewInit();
	if(GLEW_ARB_debug_output){
		glDebugMessageCallbackARB(&OpenGL_DebugMessage, nullptr);
	}



	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(SCRWIDTH, SCRHEIGHT);
	
	if (!init()) 
	{
		return 0;
	}
	int exitapp = 0;
	game = new Game();
	game->Init();
	while (!exitapp) 
	{
		// calculate frame time and pass it to game->Tick
		LARGE_INTEGER start, end;
		QueryPerformanceCounter( &start );
		swap();
		game->Tick( (float)lastftime );




		// event loop
		SDL_Event event;
		while (SDL_PollEvent( &event )) 
		{
			bool handled = TwEventSDL(&event, SDL_MAJOR_VERSION, SDL_MINOR_VERSION);
			if(!handled){
			switch (event.type)
			{
			case SDL_QUIT:
				exitapp = 1;
				break;
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) 
				{
					exitapp = 1;
					// find other keys here: http://sdl.beuc.net/sdl.wiki/SDLKey
				}
				game->KeyDown( event.key.keysym.scancode );
				break;
			case SDL_KEYUP:
				game->KeyUp( event.key.keysym.scancode );
				break;
			case SDL_MOUSEMOTION:
				game->MouseMove( event.motion.x, event.motion.y );
				break;
			case SDL_MOUSEBUTTONUP:
				game->MouseUp( event.button.button );
				break;
			case SDL_MOUSEBUTTONDOWN:
				game->MouseDown( event.button.button );
				break;
			default:
				// more info on events in SDL: http://sdl.beuc.net/sdl.wiki/SDL_Event
				break;
			}
		}
		}
		QueryPerformanceCounter( &end );
		lastftime = float((end.QuadPart - start.QuadPart ))  / (float)ticksPS.QuadPart;
	}
	TwTerminate();
	SDL_Quit();
	FreeImage_DeInitialise();
	return 1;
}