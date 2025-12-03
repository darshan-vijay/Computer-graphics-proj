#version 120

void main()
{
    vec2 uv = gl_PointCoord;
    vec2 center = uv - 0.5;
    float dist = length(center) * 2.0;  // Distance from center

    // Simple circle that fades at edges
    float circle = 1.0 - smoothstep(0.7, 1.0, dist);
    
    float alpha = circle * gl_Color.a * 0.5;

    if (alpha < 0.03)
        discard;

    gl_FragColor = vec4(0.9, 0.95, 1.0, alpha);
}
