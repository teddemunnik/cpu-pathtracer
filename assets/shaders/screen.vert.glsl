#version 330 core

in layout(location=0) vec2 in_vertexPosition;

out vec2 fragUv;
void main(){
	gl_Position = vec4(in_vertexPosition, 0.0f, 1.0f);
	fragUv = in_vertexPosition/2.0f+0.5f;
	fragUv.y = 1.0f - fragUv.y;
}