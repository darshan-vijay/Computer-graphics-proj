#version 120

attribute vec4 rainData;  // (offsetX, offsetZ, speed, length)

uniform float uTime;
uniform float uHeight;

// Wind + turbulence parameters
uniform float windStrength;
uniform float turbulenceAmp;
uniform float turbulenceFreq;
uniform float turbulenceSpeed;

void main()
{
    float offsetX = rainData.x;
    float offsetZ = rainData.y;
    float speed   = rainData.z;
    float length  = rainData.w;

    // Falling
    float fall = mod(uTime * speed, uHeight);
    float y = uHeight - fall;

    // Wind drift
    float windX = windStrength * uTime;

    // Turbulence wiggle
    float turb = turbulenceAmp *
                 sin(y * turbulenceFreq + uTime * turbulenceSpeed);

    float finalX = offsetX + windX + turb;

    vec3 pos = vec3(finalX, y, offsetZ);
    
    gl_Position = gl_ModelViewProjectionMatrix * vec4(pos, 1.0);

    // Fade near ground
    float fade = smoothstep(0.0, 3.0, y);

    // Streak length to pixel size
    gl_PointSize = 20.0 + length * 40.0;

    // Pass fade to fragment shader
    gl_FrontColor = vec4(1.0, 1.0, 1.0, fade);
}
