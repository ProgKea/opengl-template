#version 330 core

uniform float time;
uniform vec2 resolution;
in vec4 out_color;

out vec4 frag_color;

void main()
{
    // Convert screen coordinates to values between -1 and 1
    vec2 uv = (2.0 * gl_FragCoord.xy - resolution) / min(resolution.x, resolution.y);

    // Add a rotating spiral pattern to the background in green color
    float radius = length(uv);
    float angle = atan(uv.y, uv.x) + time * 10.0;
    float spiral_factor = 0.2 / sqrt(radius) * cos(angle * 8.0);
    vec3 spiral_color = vec3(0.0, 1.0, 0.0); // Green color for the spiral
    vec3 color = spiral_factor * spiral_color;

    // Create a virus bite effect using a circular mask
    float mask = smoothstep(0.5, 0.45, length(uv));
    vec3 bite_color = mix(vec3(0.0), vec3(1.0, 0.9, 0.7), mask); // Green-yellow color for the virus bite
    color += bite_color;

    // Supercharge the virus bite with a dynamic wave pattern
    float freq = mix(20.0, 50.0, fract(sin(43758.5453123 + time * 2.0)));
    float amp = mix(0.05, 0.2, fract(cos(235.2653785 + time * 3.0)));
    float dist = abs(uv.y - sin(uv.x * freq + time * 2.0) * amp);
    vec3 wave_color = vec3(fract(dist), fract(dist + 0.333), fract(dist + 0.666));
    color += wave_color;

    // Set the output color
    frag_color = vec4(color, 1.0);
}
