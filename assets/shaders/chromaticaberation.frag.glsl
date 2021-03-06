#version 330 core

uniform sampler2D _MainTex;
uniform vec2 _ScreenSize;

in vec2 fragUv;

out vec3 color;

const float offset=5.0f;

void main(){
	vec2 move = 1.0f / _ScreenSize;
	vec2 dist = fragUv - vec2(0.5f, 0.5f);
	vec2 sqrDist = dist * dist;

	vec3 c;
	c.r = texture2D(_MainTex, fragUv - sqrDist*move*offset).r;
	c.g = texture2D(_MainTex, fragUv).g;
	c.b = texture2D(_MainTex, fragUv + sqrDist*move*offset).b;
	color = c;
}