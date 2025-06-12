#version 330 core
layout (location = 0) out vec4 fragColor;

uniform vec2 u_resolution;
uniform float u_time;

#define FREQUENCY 14.0
#define CONTRAST 0.25

void main() {
    vec2  quadmirror = abs((gl_FragCoord.xy-.5*u_resolution.xy)/min(u_resolution.x, u_resolution.y))*(2.0+sin(u_time/3.14));
    float angle = 0.2*sin(u_time)/0.5*length(quadmirror);
          quadmirror = atan(quadmirror*1.5) * pow(quadmirror, vec2(sin(u_time)*.2+cos(u_time)/4.0));
          quadmirror *= mat2(vec2(sin(angle), cos(angle)), vec2(cos(angle), -sin(angle)));
         
    float inneroutter  = quadmirror.x + quadmirror.y,
          horzvert     = quadmirror.x - quadmirror.y,
          squarecenter = max(quadmirror.x, quadmirror.y);
    
    float stripes = pow(fract(
        (     (inneroutter > 1.0)  ? min(quadmirror.x, quadmirror.y)
                  // Outter most square
            : (squarecenter > 0.5) ? horzvert
                  // Middle, 45 degree square
            : -sign(horzvert)*inneroutter
                  // Inner most square
        ) * FREQUENCY *(0.9/5.0 + 0.2*cos(u_time)*float(atan(quadmirror.x, quadmirror.y)))
    ), CONTRAST);

    
    
    float borders = min(min( abs(squarecenter - 1.0 ),
                             abs(inneroutter  - 1.0 )),
                        min( abs(squarecenter - 0.5 ),
                             max( abs(horzvert), squarecenter - 0.5 )));
    
    stripes -= pow(borders, CONTRAST);
    fragColor = vec4(pow(stripes, 7.0), pow(stripes, 1.3), pow(stripes, 7.0), 1.0);
}