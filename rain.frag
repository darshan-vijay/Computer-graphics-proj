#version 120

void main()
{
    vec2 uv = gl_PointCoord;

    // Make a thin streak vertically (centered horizontally)
    float xDist = abs(uv.x - 0.5);

    // Very thin streak: width control
    float width = 0.08;   // smaller = sharper, thinner streak
    float alphaX = 1.0 - smoothstep(width, width+0.02, xDist);

    // Vertical gradient to shape the streak
    float head = smoothstep(0.0, 0.15, uv.y);   // sharp start
    float tail = smoothstep(1.0, 0.85, uv.y);   // fade end
    float alphaY = head * tail;

    float alpha = alphaX * alphaY;

    if (alpha < 0.05) discard;   // make edges precise

    gl_FragColor = vec4(0.8, 0.9, 1.0, alpha);
}
