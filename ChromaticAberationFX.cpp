#include "ChromaticAberationFX.h"
#include "Renderer.h"

ChromaticAberationFX::ChromaticAberationFX(){
	m_Shader.loadFile("assets/shaders/default.vert.glsl", "assets/shaders/chromaticaberation.frag.glsl");
	Shader::bind(&m_Shader);
	Shader::setVec2(m_Shader.uniformLocation("_ScreenSize"), float2((float)SCRWIDTH, (float)SCRHEIGHT));
}
ChromaticAberationFX::~ChromaticAberationFX(){

}
void ChromaticAberationFX::applyEffect(Renderer* renderer, GLuint in, GLuint out){
	renderer->blit(in, out, &m_Shader);
}