#pragma once
#include <gl/glew.h>
class Renderer;
class PostFX{
public:
	virtual ~PostFX()=0{};
	virtual void applyEffect(Renderer* renderer, GLuint inTexture, GLuint outFBO)=0;
};