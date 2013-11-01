#version 330 core

uniform sampler2D _MainTex;

in vec2 fragUv;

out vec3 color;
void main(){
	color = _MainTex.rgb - vec3(1.0f, 1.0f, 1.0f);
}