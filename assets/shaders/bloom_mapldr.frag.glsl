#version 330 core

uniform sampler2D _MainTex;
uniform float _MainTexSamples;

in vec2 fragUv;

out vec3 color;
void main(){
	color = min(texture2D(_MainTex, fragUv).rgb, 1.0f);

}