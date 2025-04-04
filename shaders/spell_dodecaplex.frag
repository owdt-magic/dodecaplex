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
    float cast_life  = pow(CAST_LIFE*.95, 1.5); 
    float s_time     = u_time*5;
    float c_time     = u_time/5;
          c_time     = mix(c_time, c_time*5, sin(c_time/2));

    vec2 window      = (gl_FragCoord.xy-(u_resolution/2.0))/(max(u_resolution.x, u_resolution.y));
    float wall_dist  = length(cross(SPELL_FOCUS, SPELL_HEAD-model_Coords.xyz))*0.5;
    vec4 spec_data   = max(texture(specularTextures, texture_Coords), 0.5 );
    
    float angle      = atan(window.x, window.y)+sin(u_time)*10.0;
    float radius     = pow(length(window*4.0)*(1.0+(zDepth))/cast_life/5., .5);
    
    float spell_flux = wall_dist+ sin(u_time/10.0)*10.0;
    float spiral     =  mix(sin(spell_flux*15.0f-cos(u_time)/radius-angle), 0.0f, log(2*radius));
          spiral     += mix(sin(spell_flux*15.0f-sin(u_time)/radius+angle), 0.0f, log(2*radius));
    float aura       = max(pow(spiral, .5), (.2+(0.05*cos(u_time*5.f)))/radius*2.)/max(zDepth*1.5, 1.0);;
    vec4 spec_lit    = vec4(1./max(1.0, pow(wall_dist*7.0, 3.0)))*vec4(1.0, .8, .8, 1.0);
         spec_lit    = max(spec_lit+vec4(0.1*cast_life/wall_dist), mix(spec_lit, vec4((aura)/radius)*vec4(0.5, 1.0, 1.0, 1.0), cast_life));

    mat2 cast_mat = mat2(sin(cast_life), cos(cast_life),-sin(cast_life), cos(cast_life));
    mat2 rit_mat = cast_mat*mat2(sin(c_time), cos(c_time),-sin(c_time), cos(c_time));
    vec2 rit_wind = rit_mat*window;

    vec4 perif_lighting = vec4(0);
    for (int i=2; i < 7; i++) {
         perif_lighting = max(perif_lighting, 
                texture(spellTextures, vec3(pow(-1.0, i)*sin(pow(s_time,i*.3333))*window[i%2], cos(pow(s_time,i*.3333))*window[i%2+1],1.0f))
         );
    }   
         perif_lighting /= 2*length(rit_wind);
         perif_lighting = pow(perif_lighting, vec4(.7));

    vec2 norm_window =rit_wind*3+vec2(0.5);

    vec4 cent_lighting =        texture(spellTextures, vec3((norm_window*rit_mat).yx, 1.0f));
         cent_lighting += pow(  texture(spellTextures, vec3(norm_window,         1.0f)), vec4(10.0));
         cent_lighting +=       texture(spellTextures, vec3(rit_mat*norm_window, 1.0f));
         cent_lighting = mix(cent_lighting, pow(perif_lighting, vec4(0.2, 0.2, 0.3, 1.0)), log(perif_lighting+1))*pow(cast_life, 1.4);
         cent_lighting = mix(spec_lit, pow(cent_lighting, vec4(10.0)), -pow(spec_data, vec4(2.0)));
         cent_lighting = pow(cent_lighting, vec4(.7));
    
    float curve_cast = pow(cast_life, 10.0);
    return mix(pow(color*(1.0-curve_cast/5), vec4(1.0+(curve_cast*.9))), 
                pow(cent_lighting, vec4(2.0)), 
                pow(cent_lighting*vec4(.2, .3+.1*sin(s_time), .6, 1.0), vec4(10.0)));
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
}
