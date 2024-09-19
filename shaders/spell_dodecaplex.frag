#version 410 core

in float zDepth;
in vec4 world_Coords;

uniform sampler2DArray pentagonTextures;
uniform sampler2DArray specularTextures;
uniform float u_time;
uniform vec2 u_resolution;
uniform float SPELL_LIFE;
uniform float CAST_LIFE;
uniform vec3 SPELL_HEAD;
uniform vec3 SPELL_FOCUS;

out vec4 color;


subroutine vec4 spell (vec4 color);
subroutine uniform spell currentSpell;

subroutine (spell) vec4 emptySpell(vec4 color) {
    return color;
}
subroutine (spell) vec4 castTeleportA(vec4 color) {
    vec4 spec_data   = vec4(0.5);//texture(specularTextures, textureCoord);
    float spell_dist = length(cross(SPELL_FOCUS, SPELL_HEAD-world_Coords.xyz));
    float spell_flux = spell_dist+ sin(u_time/10.0)*0.001;
    vec2 window      = (gl_FragCoord.xy-(u_resolution/2.0))/((max(u_resolution.x, u_resolution.y) * pow(.1, SPELL_LIFE))*.5);
    float angle      = atan(window.x, window.y)+sin(u_time)*10.0f;
    float radius     = length(window)*(1.0+zDepth)/CAST_LIFE;
    float spiral     =  mix(sin(spell_flux*15.0f-cos(u_time)/radius-angle), 0.0f, radius);
          spiral     += mix(sin(spell_flux*15.0f-sin(u_time)/radius+angle), 0.0f, radius);
    float aura       = max(spiral/pow(spell_dist*10.0, 10.0), (.2+(0.05*cos(u_time*5.f)))/radius*2.);
    vec4 spec_lit    = max(vec4(pow(4.0*spec_data.x, 2.0)/max(1.0, pow(spell_dist*7.0, 3.0)))*vec4(1.0, .8, .8, 1.0)*CAST_LIFE, color);
    vec4 lit2        = max(spec_lit+vec4(0.1*CAST_LIFE/spell_dist), mix(spec_lit, vec4(aura/radius)*vec4(0.5, 1.0, 1.0, 1.0), CAST_LIFE));
    return mix(lit2, spec_lit, clamp(0., 1.1-CAST_LIFE, radius));
}
subroutine (spell) vec4 releaseTeleportA(vec4 color) {
    
    float spell_dist = length(cross(SPELL_FOCUS, SPELL_HEAD-world_Coords.xyz));
    vec2 window      = (gl_FragCoord.xy-u_resolution)/u_resolution;
    float angle      = atan(window.x, window.y)+(u_time+SPELL_LIFE)*15.0f;

    return mix(color, mix(color, mix(color, vec4(spell_dist*1.4, spell_dist, spell_dist, 1.0), pow((1.0-SPELL_LIFE),1.5)), spell_dist/2.0), clamp(1.0, 0.5, SPELL_LIFE));
}

vec4 hueShifter(vec4 color, float u_theta){
    vec3 yiqColor = mat3( 0.299, 1.0, 0.40462981, 0.587, -0.46081557, -1.0, 0.114, -0.53918443, 0.59537019 ) * color.rgb;
    yiqColor.yz = mat2( cos( u_theta ), sin( u_theta ), -sin( u_theta ), cos( u_theta ) ) * yiqColor.yz;
    return  vec4(mat3( 1.0, 1.0, 1.0, 0.5696804, -0.1620848, -0.6590654, 0.3235513, -0.3381869, 0.8901581 ) * yiqColor, 1.0);
}
void main(){
    color = (sin(world_Coords)+1)/2;
    color = color/max(1.0, zDepth/1.0);
    //color = hueShifter(color, world_Coords.x/5.);
    //color = color*2.0;
    color = currentSpell(color);
}
