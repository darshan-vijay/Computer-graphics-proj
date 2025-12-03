#version 120

attribute vec3 splashData;  // (offsetX, offsetZ, birthTime)

uniform float uTime;

void main()
{
    float offsetX = splashData.x;
    float offsetZ = splashData.y;
    float birthTime = splashData.z;

    // How old is this splash?
    float age = uTime - birthTime;
    
    // Kill if too old or not born yet
    if (age < 0.0 || age > 0.4) {
        gl_PointSize = 0.0;
        return;
    }

    // Splash on ground
    vec3 pos = vec3(offsetX, 0.05, offsetZ);
    gl_Position = gl_ModelViewProjectionMatrix * vec4(pos, 1.0);

    // Simple: start small, grow bigger, fade out
    float progress = age / 0.4;  // 0 to 1
    gl_PointSize = 5.0 + progress * 30.0;  // Grows from 5 to 25
    gl_FrontColor = vec4(1.0, 1.0, 1.0, 1.0 - progress);  // Fades out
}