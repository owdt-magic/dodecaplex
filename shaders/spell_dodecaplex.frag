#version 410 core

in float zDepth;
in vec4 model_Coords;
in vec3 texture_Coords;

uniform sampler2DArray pentagonTextures;
uniform sampler2DArray specularTextures;
uniform sampler2DArray spellTextures;
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
    
    float spell_life = 0.0;
    vec3 spell_head = SPELL_HEAD;
    vec3 spell_focus = SPELL_FOCUS;
    float cast_life  = CAST_LIFE*.95;

    vec2 window      = (gl_FragCoord.xy-(u_resolution/2.0))/((max(u_resolution.x, u_resolution.y) * pow(.1, spell_life))*.5);
    float wall_dist  = length(cross(spell_focus, spell_head-model_Coords.xyz))*0.5;
    vec4 spec_data   = max(texture(specularTextures, texture_Coords), 0.5 );
    
    float angle      = atan(window.x, window.y)+sin(u_time)*10.0;
    float radius     = pow(length(window*4.0)*(1.0+(zDepth))/cast_life/5., .5);
    
    float spell_flux = wall_dist+ sin(u_time/10.0)*10.0;
    float spiral     =  mix(sin(spell_flux*15.0f-cos(u_time)/radius-angle), 0.0f, log(2*radius));
          spiral     += mix(sin(spell_flux*15.0f-sin(u_time)/radius+angle), 0.0f, log(2*radius));
    float aura       = max(pow(spiral, .5), (.2+(0.05*cos(u_time*5.f)))/radius*2.)/max(zDepth*1.5, 1.0);;
    vec4 spec_lit    = vec4(1./max(1.0, pow(wall_dist*7.0, 3.0)))*vec4(1.0, .8, .8, 1.0);
    float s_time = u_time*2;
    spec_lit        = max(spec_lit+vec4(0.1*cast_life/wall_dist), mix(spec_lit, vec4((aura)/radius)*vec4(0.5, 1.0, 1.0, 1.0), cast_life));
    
    vec4 perif_lighting = texture(spellTextures, vec3(cos(s_time)*window.x, sin(s_time)*window.y,1.0f));
    perif_lighting += texture(spellTextures, vec3(sin(s_time)*window.x, cos(s_time)*window.y,1.0f));
    perif_lighting += texture(spellTextures, vec3(-sin(s_time)*window.x, cos(s_time)*window.y,1.0f));
    
    mat2 cast_mat = mat2(sin(cast_life), cos(cast_life),-sin(cast_life), cos(cast_life));


    for (int i=2; i < 4; i++) {
        perif_lighting += texture(spellTextures, vec3(-sin(pow(s_time, i))*window.x, cos(pow(s_time,i))*window.y,1.0f));
    }    
    
    vec2 newwin = window*3;
    newwin = pow(window, vec2(0.9));

    vec2 winnorm = vec2(newwin.x+0.5, newwin.y+0.5);
    newwin = pow(window, vec2(1.14));
    vec2 winnorm2 = vec2(newwin.x+0.5, newwin.y+0.5); 
    vec4 cent_lighting = texture(spellTextures, vec3(winnorm*cast_mat, 1.0f));
        cent_lighting += pow(texture(spellTextures, vec3(winnorm2, 1.0f)), vec4(10.0));
        cent_lighting += texture(spellTextures, vec3(cast_mat*winnorm2, 1.0f));
    perif_lighting /= length(window);
    cent_lighting = max(cent_lighting, pow(perif_lighting, vec4(0.2, 0.2, 0.3, 1.0)))*pow(cast_life, 1.4);
    cent_lighting = max(spec_lit, cent_lighting);
    return max(color, pow(cent_lighting, vec4(2.0)));
}
subroutine (spell) vec4 releaseMining(vec4 color) {
    float wall_dist = length(cross(SPELL_FOCUS, SPELL_HEAD-model_Coords.xyz));
    return mix(color,
            mix(color, 
                mix(color, 
                    mix(color, vec4(1.0, 1.1, 1.3, 1.0)/sqrt(wall_dist),  pow((1.0-SPELL_LIFE)*8.0, -1.5)), 
                pow(wall_dist, 1.0)), 
            clamp(0.01, 1.0, pow(SPELL_LIFE, -2.0)/wall_dist)),            
            1.0/wall_dist);
}

void main(){
    color = texture(pentagonTextures, texture_Coords);
    color /= max(zDepth*1.5, 1.0);
    color = currentSpell(color);
    //color = castMining(color);
}
