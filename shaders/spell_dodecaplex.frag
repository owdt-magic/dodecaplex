#version 410 core

in float zDepth;
in vec4 model_Coords;
in vec3 texture_Coords;

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
subroutine (spell) vec4 castMining(vec4 color) {
    vec2 window      = (gl_FragCoord.xy-(u_resolution/2.0))/((max(u_resolution.x, u_resolution.y) * pow(.1, SPELL_LIFE))*.5);
    float wall_dist = length(cross(SPELL_FOCUS, SPELL_HEAD-model_Coords.xyz));
    vec4 spec_data   = max(texture(specularTextures, texture_Coords), 0.5f);
    
    float angle      = atan(window.x, window.y)+sin(u_time)*10.0f;
    float radius     = length(window*4.0)*(1.0+(zDepth))/CAST_LIFE;
    
    float spell_flux = wall_dist+ sin(u_time/10.0)*0.001;
    float spiral     =  mix(sin(spell_flux*15.0f-cos(u_time)/radius-angle), 0.0f, radius);
          spiral     += mix(sin(spell_flux*15.0f-sin(u_time)/radius+angle), 0.0f, radius);
    float aura       = max(spiral/2.0, (.2+(0.05*cos(u_time*5.f)))/radius*2.)/max(zDepth*1.5, 1.0);;
    vec4 spec_lit    = max(vec4(pow(4.0*spec_data.x, -1.0)/max(1.0, pow(wall_dist*7.0, 3.0)))*vec4(1.0, .8, .8, 1.0)*CAST_LIFE, color);
    vec4 lit2        = max(spec_lit+vec4(0.1*CAST_LIFE/wall_dist), mix(spec_lit, vec4((aura)/radius)*vec4(0.5, 1.0, 1.0, 1.0), CAST_LIFE));    
    
    return mix(min(lit2, color.bbbb*10.0), spec_lit, clamp(0., 1.1-CAST_LIFE, radius/length(window)));
}
subroutine (spell) vec4 releaseMining(vec4 color) {
    float wall_dist = length(cross(SPELL_FOCUS, SPELL_HEAD-model_Coords.xyz));
    return mix(color, 
                mix(color, 
                    mix(color, vec4(1.0, 1.1, 1.3, 1.0)*wall_dist,  pow((1.0-SPELL_LIFE), 1.5)), 
                pow(wall_dist, -1.0)), 
            clamp(0.01, 1.0, pow(SPELL_LIFE, 2.0)));
}

void main(){
    color = texture(pentagonTextures, texture_Coords);
    color /= max(zDepth*1.5, 1.0);
    color = currentSpell(color);
}
