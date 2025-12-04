#version 120

attribute vec3 splashData;  // (xPos, zPos, collisionTime)
//currtime
uniform float time;

void main()
{
    float xPos = splashData.x;
    float zPos = splashData.y;
    float collisionTime = splashData.z;

    // calculate the splash time, we use it to discard unwanted splash
    float age = time - collisionTime;
    
    // if greater than 0.4, means the entire animation of it growing has completed
    if (age < 0.0 || age > 0.4) {
        gl_PointSize = 0.0;
        return;
    }

    // Splash generated just above ground
    vec3 pos = vec3(xPos, 0.05, zPos);
    gl_Position = gl_ModelViewProjectionMatrix * vec4(pos, 1.0);

    // small to big
    float progress = age / 0.4;  // 0 to 1
    gl_PointSize = 5.0 + progress * 30.0;  // Grows from 5 to 25
    gl_FrontColor = vec4(1.0, 1.0, 1.0, 1.0 - progress);  // Fades out
}