#version 120

attribute vec4 rainData;  // (offsetX, offsetZ, speed, length)

uniform float uTime;
uniform float uHeight;

void main()
{
    float offsetX = rainData.x;
    float offsetZ = rainData.y;
    float speed   = rainData.z;
    float length  = rainData.w;

    // Calculate falling motion
    float fall = mod(uTime * speed, uHeight);
    float y = uHeight - fall;

    // Follow camera
    vec3 pos = vec3(offsetX, 
                    y, 
                    offsetZ);
    
    gl_Position = gl_ModelViewProjectionMatrix * vec4(pos, 1.0);

    gl_PointSize = 30.0 + length * 50.0;
    gl_FrontColor = vec4(1.0, 1.0, 1.0, 0.7);
}