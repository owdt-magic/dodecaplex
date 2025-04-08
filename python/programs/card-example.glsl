#version 330 core

layout (location = 0) out vec4 fragColor;

uniform vec2 u_resolution;
uniform float u_time;
float rectSDF(vec2 st, vec2 s) {
	return max( abs(st.x/s.x),
				abs(st.y/s.y) );
}
//--------------------------------------
float crossSDF(vec2 st, float s, float x) {
	vec2 size = vec2(x, s);
	return min( rectSDF(st, size.xy),
				rectSDF(st, size.yx) );
}
//--------------------------------------
float stroke(float x, float s, float w) {
	float d = step(s, x+w*.5) - step(s, x-w*.5);
	return clamp(d, 0., 1.);
}
float fill(float x, float size) {
	return 1.-step(size, x);
}
vec2 pR(in vec2 p, float a) {
	return cos(a)*p + sin(a)*vec2(p.y, -p.x);
}
void main() {
	vec3 color = vec3(0.);
	vec2 st = ((gl_FragCoord.xy/u_resolution)-vec2(.5));
	//--------------------------------------
	float rect = rectSDF(st, vec2(.5));
	color += fill(rect, .5);
	float cross = crossSDF(pR(st, u_time), .5*sin(u_time), min(.25*cos(u_time), .25*sin(u_time)));
	color *= step(.5, fract(cross*4.*min(sin(u_time), cos(u_time)))); // fract(1.1, 2.1, 1.6, 2.6) -> (.1, .1, .6, .6) -> step() -> (.1, .1)
	color *= step(1., cross);  
	color += fill(cross, .5); // inner while +
	color += stroke(rect, .65, .05); // 2nd outer square
	color += stroke(rect, .75, .025);
	//--------------------------------------
    fragColor = vec4(color, 1.);
}
