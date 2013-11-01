#include "Renderer.h"
#include "template.h"
#include "PostFX.h"
#include "BloomFX.h"
#include "ChromaticAberationFX.h"

const static GLfloat g_FullScreenVboData[] = {
	-1.0f, 1.0f,
	-1.0f, -1.0f,
	1.0f, -1.0f,
	-1.0f, 1.0f,
	1.0f, -1.0f,
	1.0f, 1.0f
};

Renderer::Renderer() :
	m_Tracer(nullptr){
	
}
Renderer::~Renderer(){
	destroy();
}
void Renderer::init(){
	//Setup VBO for full screen rendering
	glGenBuffers(1, &m_FullscreenVbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_FullscreenVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_FullScreenVboData), g_FullScreenVboData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//Setup framebuffers and textures for applying HDR post Fx
	glGenTextures(2, m_FrameBufferTextures);
	glActiveTexture(GL_TEXTURE0);
	for(int i=0; i<2; ++i){
		glBindTexture(GL_TEXTURE_2D, m_FrameBufferTextures[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCRWIDTH, SCRHEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(2, m_FrameBuffers);
	for(int i=0; i<2; ++i){
		glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffers[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_FrameBufferTextures[i], 0);
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if(status != GL_FRAMEBUFFER_COMPLETE){
			printf("Renderer failed to setup frame buffer %i.\n", i);
			break;
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Load default shader(no post fx)
	m_DefaultShader.loadFile("assets/shaders/screen.vert.glsl", "assets/shaders/default.frag.glsl");
	m_DefaultShaderMainTex = m_DefaultShader.uniformLocation("_MainTex");
	m_DefaultShaderSamples = m_DefaultShader.uniformLocation("_MainTexSamples");

	//Post FX
	//BloomFX* bloom = new BloomFX;
	//m_Effects.push_back(bloom);

	//ChromaticAberationFX* chromab = new ChromaticAberationFX;
	//m_Effects.push_back(chromab);
}
void Renderer::destroy(){
	glDeleteTextures(2, m_FrameBufferTextures);
	glDeleteFramebuffers(2, m_FrameBuffers);
	glDeleteBuffers(1, &m_FullscreenVbo);
}
void Renderer::renderFullscreen(){
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, m_FullscreenVbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(0);
}
void Renderer::render(){
	//Perform pathtracing
	m_Tracer->render();
	
	//If there are post processing effects render to a framebuffer, else render directly to screen
	if(m_Effects.size() > 0){
		glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffers[1]);
	}else{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	Shader::bind(&m_DefaultShader);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_FrameBufferTextures[0]);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, SCRWIDTH, SCRHEIGHT, GL_RGBA, GL_FLOAT, m_Tracer->getAccumulationBuffer());
	Shader::setInt(m_DefaultShaderMainTex, 0);
	Shader::setFloat(m_DefaultShaderSamples, (float)m_Tracer->getAccumulationBufferCount());
	renderFullscreen();

	if(m_Effects.size() > 0){
		//Render effects alternating between the two framebuffers
		int fromId=1,  toId=0;
		for(size_t i=0; i<m_Effects.size()-1; ++i){
			m_Effects[i]->applyEffect(this, m_FrameBufferTextures[fromId], m_FrameBuffers[toId]);
			std::swap(fromId, toId);
		}
		//Final effect renders directly to screen
		m_Effects[m_Effects.size()-1]->applyEffect(this, m_FrameBufferTextures[fromId], 0);
	}
}
void Renderer::blit(GLuint inTexture, GLuint outFBO, Shader* shader){
	glBindFramebuffer(GL_FRAMEBUFFER, outFBO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, inTexture);
	Shader::bind(shader);
	GLint location = shader->uniformLocation("_MainTex");
	shader->setInt(location, 0);
	renderFullscreen();
}