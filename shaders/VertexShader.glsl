#version 430

// Input
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;

// Uniform properties
uniform mat4 Model;

uniform float time;
uniform unsigned int gerstner_waves_length;
uniform struct GerstnerWave {
    vec2 direction;
    float amplitude;
    float steepness;
    float frequency;
    float speed;
    vec2 padding;
} gerstner_waves[9];

// Output
out int instance;

vec3 gerstner_wave_normal(vec3 position, float time) {
    vec3 wave_normal = vec3(0.0, 1.0, 0.0);
    for (uint i = 0; i < gerstner_waves_length; ++i) {
        float proj = dot(position.xz, gerstner_waves[i].direction),
              phase = time * gerstner_waves[i].speed,
              psi = proj * gerstner_waves[i].frequency + phase,
              Af = gerstner_waves[i].amplitude *
                   gerstner_waves[i].frequency,
              alpha = Af * sin(psi);

        wave_normal.y -= gerstner_waves[i].steepness * alpha;

        float x = gerstner_waves[i].direction.x,
              y = gerstner_waves[i].direction.y,
              omega = Af * cos(psi);

        wave_normal.x -= x * omega;
        wave_normal.z -= y * omega;
    } return wave_normal;
}

vec3 gerstner_wave_position(vec2 position, float time) {
    vec3 wave_position = vec3(position.x, 0, position.y);
    for (uint i = 0; i < gerstner_waves_length; ++i) {
        float proj = dot(position, gerstner_waves[i].direction),
              phase = time * gerstner_waves[i].speed,
              theta = proj * gerstner_waves[i].frequency + phase,
              height = gerstner_waves[i].amplitude * sin(theta);

        wave_position.y += height;

        float maximum_width = gerstner_waves[i].steepness *
                              gerstner_waves[i].amplitude,
              width = maximum_width * cos(theta),
              x = gerstner_waves[i].direction.x,
              y = gerstner_waves[i].direction.y;

        wave_position.x += x * width;
        wave_position.z += y * width;
    } return wave_position;
}

vec3 gerstner_wave(vec2 position, float time) {
    vec3 wave_position = gerstner_wave_position(position, time);
    //normal = gerstner_wave_normal(wave_position, time);
    return wave_position; // Accumulated Gerstner Wave.
}

void main()
{
    instance = gl_InstanceID;
    gl_Position =  Model * vec4(gerstner_wave(v_position.xz, time), 1);
}
