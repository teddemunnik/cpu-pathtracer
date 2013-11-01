#pragma once
#include "PostFX.h"
#include "Shader.h"

class ChromaticAberationFX : public PostFX{
private:
	Shader m_Shader;

public:
	ChromaticAberationFX();
	~ChromaticAberationFX();
	void applyEffect(Renderer* renderer, GLuint source, GLuint dest);
};