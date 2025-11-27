#version 120

void main()
{
    vec2 uv = gl_PointCoord;

    // Horizontal thinness
    float xDist = abs(uv.x - 0.5);

    // Pixel-dependent thinness (ensures crisp streaks)
    float px = fwidth(uv.x);
    float width = px * 0.4;

    float alphaX = 1.0 - smoothstep(width, width * 3.0, xDist);

    // Vertical gradient (head = bright, tail = fade)
    float head = smoothstep(0.0, 0.1, uv.y);
    float tail = smoothstep(1.0, 0.9, uv.y);

    float alpha = alphaX * head * tail * gl_Color.a;

    if (alpha < 0.03)
        discard;

    vec3 rainColor = vec3(0.75, 0.85, 1.0);
    gl_FragColor = vec4(rainColor, alpha);
}
