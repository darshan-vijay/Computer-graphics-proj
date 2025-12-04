#version 120

attribute vec4 rainData;  // (xPos, yPos, speed, length)

uniform float currTime;
uniform float height;

void main()
{
    float xPos = rainData.x;
    float yPos = rainData.y;
    float speed   = rainData.z;
    float length  = rainData.w;

    // Calculate fall
    float fall = mod(currTime * speed, height);
    float y = height - fall;

    // get the position
    vec3 pos = vec3(xPos, 
                    y, 
                    yPos);
    //calculate the position of the drop
    gl_Position = gl_ModelViewProjectionMatrix * vec4(pos, 1.0);
    //get the sprite square width using pointSize
    gl_PointSize = 30.0 + length * 50.0;
    // set the alpha and color, to be available for frag shader
    gl_FrontColor = vec4(1.0, 1.0, 1.0, 0.7);
}