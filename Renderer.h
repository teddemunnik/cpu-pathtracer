#pragma once
#include <GL/glew.h>
#include <vector>
#include "Shader.h"
#include "raytracer.h"

class PostFX;
class Renderer{
private:
	//Front/Back framebuffers for applying post fx
	GLuint m_FrameBuffers[2];
	GLuint m_FrameBufferTextures[2];
	//Fullscreen VBO to render something to entire screen
	GLuint m_FullscreenVbo;
	
	Shader m_DefaultShader;
	GLint m_DefaultShaderMainTex;
	GLint m_DefaultShaderSamples;
	
	Tracer* m_Tracer;

	std::vector<PostFX*> m_Effects;


	void renderFullscreen();
public:
	Renderer();
	~Renderer();
	void init();
	void destroy();
	void render();
	void blit(GLuint inTexture, GLuint outFbo, Shader* shader);

	void setTracer(Tracer* tracer){ m_Tracer = tracer; }
};