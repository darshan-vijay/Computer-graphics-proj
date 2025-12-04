#version 120

void main()
{
    // Every pixel in the point sprite
    vec2 uv = gl_PointCoord;

    // make the centres light up
    float xDist = abs(uv.x - 0.5); //center distance. center = 0.0
    float alphaX = 1.0 - smoothstep(0.01, 0.02, xDist); // light up the center

    // Vertically the start of the drop must be transperant 
    float tail = smoothstep(0.0, 0.4, uv.y);

    // calculate final alpha value for the point
    float alpha = alphaX * tail * gl_Color.a;

    // if very small discard
    if (alpha < 0.03)
        discard;
    //output the color for the pixel
    vec3 rainColor = vec3(0.9, 0.90, 1.0);
    gl_FragColor = vec4(rainColor, alpha * 0.8);
}