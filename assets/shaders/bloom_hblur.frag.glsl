#version 330 core

uniform sampler2D _MainTex;
uniform vec2 _ScreenSize;

in vec2 fragUv;

out vec3 color;

void main(){
	float blurSize = 1.0f/_ScreenSize.x;
	vec3 sum = vec3(0.0f, 0.0f, 0.0f);
	sum += texture2D(_MainTex, vec2(fragUv.x - 4.0f * blurSize, fragUv.y)).rgb * 0.0162162162;
	sum += texture2D(_MainTex, vec2(fragUv.x - 3.0f * blurSize, fragUv.y)).rgb * 0.0540540541;
	sum += texture2D(_MainTex, vec2(fragUv.x - 2.0f * blurSize, fragUv.y)).rgb * 0.1216216216;
	sum += texture2D(_MainTex, vec2(fragUv.x - blurSize, fragUv.y)).rgb * 0.1945945946;
	sum += texture2D(_MainTex, fragUv).rgb * 0.2270270270;
	sum += texture2D(_MainTex, vec2(fragUv.x + blurSize, fragUv.y)).rgb * 0.1945945946;
	sum += texture2D(_MainTex, vec2(fragUv.x + 2.0f * blurSize, fragUv.y)).rgb * 0.1216216216;
	sum += texture2D(_MainTex, vec2(fragUv.x + 3.0f * blurSize, fragUv.y)).rgb * 0.0540540541;
	sum += texture2D(_MainTex, vec2(fragUv.x + 4.0f * blurSize, fragUv.y)).rgb *  0.0162162162;

	color = sum;
}