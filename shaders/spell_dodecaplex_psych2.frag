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

float psychedelicIntensity = 0.5;

// Calculate luminance from RGB
float calculateLuminance(vec3 color) {
    return pow(dot(color, vec3(0.299, 0.587, 0.114)), 2.0);
}

// Map luminance to psychedelic intensity
float luminanceToIntensity(float luminance) {
    // Create a non-linear mapping where darker areas get more intense effects
    float inverted = 1.0 - luminance;
    return pow(inverted, 1.5) *3.0; // Amplify the effect for dark areas
}

// Adaptive psychedelic intensity based on texture luminance
float getAdaptiveIntensity(vec4 textureColor) {
    float luminance = calculateLuminance(textureColor.rgb);
    float baseIntensity = psychedelicIntensity;
    float luminanceIntensity = luminanceToIntensity(luminance);
    
    // Combine base intensity with luminance-based intensity
    return baseIntensity + luminanceIntensity * 0.8;
}

// Psychedelic utility functions
vec3 kaleidoscope(vec3 color, float intensity) {
    float angle = atan(color.y, color.x) + u_time * 0.5;
    float radius = length(color.xy);
    angle = mod(angle, 6.28318 / 6.0) * 6.0; // 6-fold symmetry
    return vec3(cos(angle) * radius, sin(angle) * radius, color.z);
}

vec3 chromaticAberration(vec3 color, float intensity) {
    float offset = intensity * 0.01;
    return vec3(
        color.r + sin(u_time * 2.0) * offset,
        color.g + sin(u_time * 2.0 + 2.094) * offset,
        color.b + sin(u_time * 2.0 + 4.188) * offset
    );
}

vec3 rainbowShift(vec3 color, float time) {
    float hue = time * 0.5 + length(color.xyz) * 2.0;
    vec3 rainbow = vec3(
        sin(hue) * 0.5 + 0.5,
        sin(hue + 2.094) * 0.5 + 0.5,
        sin(hue + 4.188) * 0.5 + 0.5
    );
    return mix(color, rainbow, 0.7);
}

vec3 fractalNoise(vec3 pos, float time) {
    float noise = 0.0;
    float amplitude = 1.0;
    float frequency = 1.0;
    
    for(int i = 0; i < 4; i++) {
        noise += sin(dot(pos * frequency, vec3(12.9898, 78.233, 45.164)) + time) * amplitude;
        amplitude *= 0.5;
        frequency *= 2.0;
    }
    
    return vec3(noise * 0.5 + 0.5);
}

// Subtle texture sampling fluctuations
vec2 textureFluctuation(vec2 uv, float time, float depth) {
    float fluctuation = sin(time * 1.3 + uv.x * 8.0) * 0.02;
    fluctuation += cos(time * 0.9 + uv.y * 6.0) * 0.015;
    fluctuation += sin(time * 2.1 + depth * 3.0) * 0.01;
    
    // Add depth-based breathing effect
    float breathing = sin(time * 0.7 + depth * 2.0) * 0.005;
    
    return uv + vec2(fluctuation, breathing);
}

// Luminance-aware texture fluctuation
vec2 luminanceFluctuation(vec2 uv, float time, float depth, float luminance) {
    // Base fluctuation
    vec2 baseFluctuation = textureFluctuation(uv, time, depth);
    
    // Luminance-based modulation
    float luminanceMod = 1.0 + (1.0 - luminance) * 0.5; // Darker areas get more fluctuation
    
    // Add luminance-specific breathing
    float luminanceBreathing = sin(time * 1.1 + luminance * 4.0) * 0.003 * luminanceMod;
    
    return baseFluctuation + vec2(luminanceBreathing, luminanceBreathing * 0.7);
}

// Specular-based displacement calculations
vec2 specularDisplacement(vec2 uv, float time, float depth) {
    // Sample specular texture for displacement data
    vec4 specData = texture(specularTextures, vec3(uv, texture_Coords.z));
    
    // Extract displacement from specular channels
    float specX = specData.r - 0.5; // Center around 0
    float specY = specData.g - 0.5;
    float specIntensity = specData.b; // Use blue channel for intensity
    float specAlpha = specData.a; // Use alpha for additional modulation
    
    // Create time-based displacement
    float timeDisplacement = sin(time * 2.0 + depth * 3.0) * 0.02;
    
    // Combine specular data with time-based effects
    vec2 displacement = vec2(
        specX * specIntensity * 0.1 + timeDisplacement,
        specY * specIntensity * 0.1 + timeDisplacement * 0.7
    );
    
    // Add specular-based breathing effect
    float specBreathing = sin(time * 1.5 + specIntensity * 5.0) * 0.005 * specAlpha;
    displacement += vec2(specBreathing, specBreathing * 0.8);
    
    return displacement;
}

// Enhanced specular overlay with displacement
vec3 specularOverlay(vec3 color, vec2 uv, float time, float depth) {
    // Sample specular texture with displacement
    vec2 displacedUV = uv + specularDisplacement(uv, time, depth);
    vec4 specData = texture(specularTextures, vec3(displacedUV, texture_Coords.z));
    
    // Create specular-based color modulation
    vec3 specColor = vec3(
        specData.r * 1.2,
        specData.g * 0.8,
        specData.b * 1.5
    );
    
    // Add specular-based psychedelic effects
    float specIntensity = length(specData.rgb);
    float specPhase = sin(time * 3.0 + specIntensity * 4.0) * 0.5 + 0.5;
    
    // Create displacement-like visual effects
    vec3 displacementEffect = vec3(
        sin(specData.r * 10.0 + time * 2.0) * 0.1,
        cos(specData.g * 8.0 + time * 1.5) * 0.1,
        sin(specData.b * 12.0 + time * 2.5) * 0.1
    );
    
    // Combine with original color
    color = mix(color, specColor, specIntensity * 0.3);
    color += displacementEffect * specPhase * 0.2;
    
    return color;
}

// Dynamic depth-based shadow calculation
vec3 depthShadows(vec3 color, float depth, float time) {
    // Create shadow based on depth and time
    float shadowIntensity = 1.0 - pow(depth * 0.3, 1.5);
    
    // Add time-based shadow movement
    float shadowPhase = sin(time * 0.8 + depth * 4.0) * 0.3 + 0.7;
    shadowIntensity *= shadowPhase;
    
    // Create depth-based color shift
    float depthShift = sin(depth * 2.0 + time * 1.2) * 0.1;
    vec3 shadowColor = vec3(
        0.8 + depthShift,
        0.9 + depthShift * 0.5,
        1.0 + depthShift * 0.3
    );
    
    return color * shadowIntensity * shadowColor;
}

// Dynamic depth implications
vec3 depthDynamics(vec3 color, float depth, float time) {
    // Depth-based color temperature
    float temperature = sin(depth * 3.0 + time * 0.6) * 0.2 + 0.8;
    color.r *= temperature;
    color.b *= 2.0 - temperature;
    
    // Depth-based saturation
    float saturation = cos(depth * 2.5 + time * 1.1) * 0.3 + 0.7;
    float luminance = dot(color, vec3(0.299, 0.587, 0.114));
    color = mix(vec3(luminance), color, saturation);
    
    // Depth-based contrast
    float contrast = sin(depth * 4.0 + time * 0.9) * 0.2 + 1.0;
    color = (color - 0.5) * contrast + 0.5;
    
    return color;
}

subroutine vec4 spell (vec4 color);
subroutine uniform spell currentSpell;

subroutine (spell) vec4 emptySpell(vec4 color) {
    // Calculate adaptive intensity based on current color luminance
    float adaptiveIntensity = getAdaptiveIntensity(color);
    
    // Apply basic psychedelic effects even in empty spell
    vec3 pos = model_Coords.xyz;
    vec3 noise = fractalNoise(pos, u_time);
    color.rgb = mix(color.rgb, noise, adaptiveIntensity * 0.3);
    color.rgb = rainbowShift(color.rgb, u_time);
    color.rgb = chromaticAberration(color.rgb, adaptiveIntensity);
    
    // Apply specular overlay with displacement effects
    color.rgb = specularOverlay(color.rgb, texture_Coords.xy, u_time, zDepth);
    
    // Apply depth-based effects
    color.rgb = depthShadows(color.rgb, zDepth, u_time);
    color.rgb = depthDynamics(color.rgb, zDepth, u_time);
    
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
    vec4 result = mix(pow(color*(1.0-curve_cast/5), vec4(1.0+(curve_cast*.9))), 
                pow(cent_lighting, vec4(2.0)), 
                pow(cent_lighting*vec4(.2, .3+.1*sin(s_time), .6, 1.0), vec4(10.0)));
    
    // Calculate adaptive intensity based on result luminance
    float adaptiveIntensity = getAdaptiveIntensity(result);
    
    // Apply psychedelic enhancements
    // Kaleidoscope effect
    result.rgb = kaleidoscope(result.rgb, adaptiveIntensity);
    
    // Enhanced rainbow shifting
    result.rgb = rainbowShift(result.rgb, u_time * 2.0);
    
    // Fractal noise overlay
    vec3 noise = fractalNoise(model_Coords.xyz, u_time * 3.0);
    result.rgb = mix(result.rgb, noise, adaptiveIntensity * 0.4);
    
    // Chromatic aberration
    result.rgb = chromaticAberration(result.rgb, adaptiveIntensity * 2.0);
    
    // Hyperbolic color distortion
    float hyperDist = length(model_Coords.xyz) * 0.5;
    vec3 hyperColor = vec3(
        sin(hyperDist + u_time) * 0.5 + 0.5,
        sin(hyperDist + u_time + 2.094) * 0.5 + 0.5,
        sin(hyperDist + u_time + 4.188) * 0.5 + 0.5
    );
    result.rgb = mix(result.rgb, hyperColor, adaptiveIntensity * 0.6);
    
    // Apply specular displacement effects for casting
    vec2 castSpecularOffset = specularDisplacement(texture_Coords.xy, u_time * 1.2, zDepth);
    result.rgb = specularOverlay(result.rgb, texture_Coords.xy + castSpecularOffset, u_time, zDepth);
    
    // Apply depth-based effects for casting
    result.rgb = depthShadows(result.rgb, zDepth, u_time);
    result.rgb = depthDynamics(result.rgb, zDepth, u_time);
    
    // Cast-specific depth breathing effect
    float castBreathing = sin(u_time * 2.0 + zDepth * 3.0) * 0.2 + 0.8;
    result.rgb *= castBreathing;
    
    return result;
}
subroutine (spell) vec4 releaseMining(vec4 color) {
    float wall_dist = length(cross(SPELL_FOCUS, SPELL_HEAD-model_Coords.xyz));
    vec4 result = mix(color,
            mix(color, 
                mix(color, 
                    mix(color, vec4(1.0, 1.1, 1.3, 1.0)/sqrt(wall_dist),  pow((1.0-SPELL_LIFE)*8.0, -1.5)), 
                pow(wall_dist, 1.0)), 
            clamp(0.01, 1.0, pow(SPELL_LIFE, -2.0)/wall_dist)),            
            1.0/wall_dist);
    
    // Calculate adaptive intensity based on result luminance
    float adaptiveIntensity = getAdaptiveIntensity(result);
    
    // Apply psychedelic enhancements
    // Explosive rainbow effect
    float explosion = pow(1.0 - SPELL_LIFE, 3.0);
    vec3 explosionColor = vec3(
        sin(u_time * 10.0 + explosion * 10.0) * 0.5 + 0.5,
        sin(u_time * 10.0 + explosion * 10.0 + 2.094) * 0.5 + 0.5,
        sin(u_time * 10.0 + explosion * 10.0 + 4.188) * 0.5 + 0.5
    );
    result.rgb = mix(result.rgb, explosionColor, explosion * adaptiveIntensity);
    
    // Spiral distortion
    vec2 spiral = vec2(
        cos(u_time * 5.0 + wall_dist * 2.0),
        sin(u_time * 5.0 + wall_dist * 2.0)
    );
    result.rgb = mix(result.rgb, vec3(spiral, 0.5), adaptiveIntensity * 0.3);
    
    // Fractal explosion
    vec3 fractal = fractalNoise(model_Coords.xyz * 2.0, u_time * 5.0);
    result.rgb = mix(result.rgb, fractal, explosion * adaptiveIntensity * 0.8);
    
    // Apply specular displacement effects for release
    vec2 releaseSpecularOffset = specularDisplacement(texture_Coords.xy, u_time * 2.0, zDepth);
    result.rgb = specularOverlay(result.rgb, texture_Coords.xy + releaseSpecularOffset, u_time, zDepth);
    
    // Apply depth-based effects for release
    result.rgb = depthShadows(result.rgb, zDepth, u_time);
    result.rgb = depthDynamics(result.rgb, zDepth, u_time);
    
    // Release-specific depth pulsing effect
    float releasePulse = sin(u_time * 8.0 + zDepth * 4.0) * 0.3 + 0.7;
    result.rgb *= releasePulse;
    
    // Depth-based explosion intensity
    float depthExplosion = pow(zDepth, 0.5) * explosion;
    result.rgb = mix(result.rgb, explosionColor, depthExplosion * adaptiveIntensity);
    
    return result;
}

void main(){
    float remainder = texture_Coords.z- floor(texture_Coords.z);
    
    // Get initial texture sample to calculate luminance
    vec4 initialV1 = texture(pentagonTextures, vec3(texture_Coords.xy, ceil(texture_Coords.z)));
    vec4 initialV2 = texture(pentagonTextures, vec3(texture_Coords.xy, floor(texture_Coords.z)));
    vec4 initialColor = mix(initialV1, initialV2, pow(remainder, 2));
    initialColor = mix(initialColor, mix(initialV1, initialV2, 1-pow(remainder, 3)), 1-remainder);
    
    // Calculate luminance from initial sample
    float luminance = calculateLuminance(initialColor.rgb);
    
    // Apply luminance-aware texture sampling fluctuations
    vec2 fluctuatedUV = luminanceFluctuation(texture_Coords.xy, u_time, zDepth, luminance);
    
    // Add specular-based displacement to UV coordinates
    vec2 specularOffset = specularDisplacement(texture_Coords.xy, u_time, zDepth);
    vec2 displacedUV = fluctuatedUV + specularOffset;
    
    vec4 v1, v2;
    v1 = texture(pentagonTextures, vec3(displacedUV, ceil(texture_Coords.z)));
    v2 = texture(pentagonTextures, vec3(displacedUV, floor(texture_Coords.z)));
    color = mix(v1, v2, pow(remainder, 2));
    color = mix(color, mix(v1, v2, 1-pow(remainder, 3)), 1-remainder);
    
    // Apply specular overlay with displacement effects
    color.rgb = specularOverlay(color.rgb, texture_Coords.xy, u_time, zDepth);
    
    // Calculate adaptive intensity based on texture luminance
    float adaptiveIntensity = getAdaptiveIntensity(color);
    
    // Apply depth-based dynamics before depth division
    color.rgb = depthDynamics(color.rgb, zDepth, u_time);
    
    // Apply depth-based shadows
    color.rgb = depthShadows(color.rgb, zDepth, u_time);
    
    // Enhanced depth division with dynamic implications
    float dynamicDepth = zDepth * (1.0 + sin(u_time * 0.5 + zDepth * 2.0) * 0.3);
    color /= max(dynamicDepth * 1.5, 1.0);
    
    // Apply base psychedelic effects with adaptive intensity
    // UV distortion for warping effect
    vec2 distortedUV = texture_Coords.xy;
    distortedUV += sin(u_time * 2.0 + distortedUV.x * 10.0) * 0.1 * adaptiveIntensity;
    distortedUV += cos(u_time * 1.5 + distortedUV.y * 8.0) * 0.1 * adaptiveIntensity;
    
    // Add specular-based displacement to psychedelic distortion
    vec2 specularPsychedelicOffset = specularDisplacement(texture_Coords.xy, u_time * 1.5, zDepth);
    distortedUV += specularPsychedelicOffset * adaptiveIntensity * 0.5;
    
    // Re-sample textures with distortion
    v1 = texture(pentagonTextures, vec3(distortedUV, ceil(texture_Coords.z)));
    v2 = texture(pentagonTextures, vec3(distortedUV, floor(texture_Coords.z)));
    vec4 distortedColor = mix(v1, v2, pow(remainder, 2));
    distortedColor = mix(distortedColor, mix(v1, v2, 1-pow(remainder, 3)), 1-remainder);
    
    // Apply additional specular overlay to distorted color
    distortedColor.rgb = specularOverlay(distortedColor.rgb, distortedUV, u_time, zDepth);
    
    // Mix original and distorted with adaptive intensity
    color = mix(color, distortedColor, adaptiveIntensity * 0.5);
    
    // Add fractal noise overlay with adaptive intensity
    vec3 noise = fractalNoise(model_Coords.xyz, u_time);
    color.rgb = mix(color.rgb, noise, adaptiveIntensity * 0.2);
    
    // Apply kaleidoscope effect to texture coordinates with adaptive intensity
    vec3 kaleidoscopeUV = kaleidoscope(vec3(texture_Coords.xy, 0.0), adaptiveIntensity);
    
    // Add specular displacement to kaleidoscope coordinates
    vec2 kaleidoscopeSpecularOffset = specularDisplacement(kaleidoscopeUV.xy, u_time * 0.8, zDepth);
    vec2 displacedKaleidoscopeUV = kaleidoscopeUV.xy + kaleidoscopeSpecularOffset * adaptiveIntensity * 0.3;
    
    vec4 kaleidoscopeColor = texture(pentagonTextures, vec3(displacedKaleidoscopeUV, texture_Coords.z));
    
    // Apply specular overlay to kaleidoscope color
   // kaleidoscopeColor.rgb = specularOverlay(kaleidoscopeColor.rgb, displacedKaleidoscopeUV, u_time, zDepth);
    
    color = mix(color, kaleidoscopeColor, adaptiveIntensity * 0.3);
    
    // Final depth-based color temperature adjustment
    float finalTemperature = sin(zDepth * 1.5 + u_time * 0.8) * 0.1 + 0.9;
    color.rgb *= vec3(finalTemperature, 1.0, 2.0 - finalTemperature);
    color /= exp(initialColor*100.);
    //color *= initialV2;
    color = currentSpell(color);
    color *= 1.0+log(initialColor);
}
