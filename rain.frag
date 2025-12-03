#version 120

void main()
{
    vec2 uv = gl_PointCoord;

    // Horizontal thinness
    float xDist = abs(uv.x - 0.5);
    float alphaX = 1.0 - smoothstep(0.01, 0.02, xDist);

    // Vertical gradient (head = bright, tail = fade)
    float tail = smoothstep(0.0, 0.4, uv.y);

    float alpha = alphaX * tail * gl_Color.a;

    if (alpha < 0.03)
        discard;

    vec3 rainColor = vec3(0.9, 0.90, 1.0);
    gl_FragColor = vec4(rainColor, alpha * 0.8);
}