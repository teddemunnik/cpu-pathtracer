#include "ChromaticAberationFX.h"
#include "Renderer.h"

ChromaticAberationFX::ChromaticAberationFX(){
	m_Shader.loadFile("assets/shaders/default.vert.glsl", "assets/shaders/chromaticaberation.frag.glsl");
}
ChromaticAberationFX::~ChromaticAberationFX(){

}
void ChromaticAberationFX::applyEffect(Renderer* renderer, GLuint in, GLuint out){
	renderer->blit(in, out, &m_Shader);
}