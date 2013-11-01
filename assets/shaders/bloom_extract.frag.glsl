#version 330 core

uniform sampler2D _MainTex;

in vec2 fragUv;

out vec3 color;
void main(){
	color = max(texture2D(_MainTex, fragUv).rgb - vec3(1.0f, 1.0f, 1.0f).rgb, 0.0f);
}