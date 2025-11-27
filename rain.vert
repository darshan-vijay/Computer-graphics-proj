#version 120

attribute vec4 rainData;
// rainData = (offsetX, offsetZ, speed, length)

uniform float uTime;
uniform float uHeight;

void main()
{
    float y = mod(uTime * rainData.z, uHeight);

    vec3 worldPos = vec3(
        rainData.x,
        uHeight - y,
        rainData.y
    );

    gl_Position = gl_ModelViewProjectionMatrix * vec4(worldPos, 1.0);

    // map world length → pixel size
    gl_PointSize = rainData.w * 90.0; 
}
