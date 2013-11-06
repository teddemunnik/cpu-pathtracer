#include "BloomFX.h"
#include "Renderer.h"

BloomFX::BloomFX(){

	//Setup render texture for bright parts to bloom
	{
		glGenTextures(1, &m_BloomRenderTexture);
		glBindTexture(GL_TEXTURE_2D, m_BloomRenderTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCRWIDTH, SCRHEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenFramebuffers(1, &m_BloomFramebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, m_BloomFramebuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_BloomRenderTexture, 0);
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if(status != GL_FRAMEBUFFER_COMPLETE){
			printf("Bloom effect failed to create framebuffer.\n");
			return;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	{
		glGenTextures(1, &m_TmpRenderTexture);
		glBindTexture(GL_TEXTURE_2D, m_TmpRenderTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCRWIDTH, SCRHEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenFramebuffers(1, &m_TmpFramebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, m_TmpFramebuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TmpRenderTexture, 0);
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if(status != GL_FRAMEBUFFER_COMPLETE){
			printf("Bloom effect failed to create framebuffer.\n");
			return;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	m_LDRMapShader.loadFile("assets/shaders/default.vert.glsl", "assets/shaders/bloom_mapldr.frag.glsl");
	m_ExtractBrightShader.loadFile("assets/shaders/default.vert.glsl", "assets/shaders/bloom_extract.frag.glsl");
	m_HorizontalGaussian.loadFile("assets/shaders/default.vert.glsl", "assets/shaders/bloom_hblur.frag.glsl");
	Shader::bind(&m_HorizontalGaussian);
	Shader::setVec2(m_HorizontalGaussian.uniformLocation("_ScreenSize"), float2((float)SCRWIDTH, (float)SCRHEIGHT));
	m_VerticalGaussian.loadFile("assets/shaders/default.vert.glsl", "assets/shaders/bloom_vblur.frag.glsl");
	Shader::bind(&m_VerticalGaussian);
	Shader::setVec2(m_HorizontalGaussian.uniformLocation("_ScreenSize"), float2((float)SCRWIDTH, (float)SCRHEIGHT));
}
BloomFX::~BloomFX(){
	glDeleteFramebuffers(1, &m_BloomFramebuffer);
	glDeleteFramebuffers(1, &m_TmpFramebuffer);
	glDeleteTextures(1, &m_BloomRenderTexture);
	glDeleteTextures(1, &m_TmpRenderTexture);
}
void BloomFX::applyEffect(Renderer* renderer, GLuint inTexture, GLuint outFBO){
	//Extract bright colors in bloom framebuffer
	renderer->blit(inTexture, m_BloomFramebuffer, &m_ExtractBrightShader);
	//blur bright colors horizontally
	renderer->blit(m_BloomRenderTexture, m_TmpFramebuffer, &m_HorizontalGaussian);


	//Base color LDR mapped colors
	renderer->blit(inTexture, outFBO, &m_LDRMapShader);

	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_BLEND);

	//Add blurred HDR colors
	renderer->blit(m_TmpRenderTexture, outFBO, &m_VerticalGaussian);
	glDisable(GL_BLEND);
}