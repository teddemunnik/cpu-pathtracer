#pragma once
#include "PostFX.h"
#include "Shader.h"
class BloomFX : public PostFX{
private:
	Shader m_LDRMapShader;
	Shader m_ExtractBrightShader;
	Shader m_HorizontalGaussian;
	Shader m_VerticalGaussian;

	//Framebuffer/rendertexture for extracted bright parts
	GLuint m_BloomRenderTexture;
	GLuint m_BloomFramebuffer;

	GLuint m_TmpRenderTexture;
	GLuint m_TmpFramebuffer;

public:
	BloomFX();
	~BloomFX();
	void applyEffect(Renderer* renderer, GLuint inTexture, GLuint outFBO);
};