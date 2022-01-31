#version 460

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texture;
layout (location = 3) flat in uint in_texture_index;
//layout (location = 4) flat in uint in_base_instance;

struct Light {
    vec4 position;
    vec4 color;
};

layout (set = 0, binding = 0) uniform RenderConstants {
    Light lights[1024];
    uint light_count;
};

layout (location = 0) out vec4 out_color;

void main() {
    const vec3 normal_color = (in_normal + 1.0f) / 2.0f;
    //const vec3 normal_color = vec3(in_texture, 1.0f);
    //vec4 color;

    //out_color = vec4(normal_color, 1.0f);;

    //const Light light[3] = {
    //    {vec3(0.0f, 0.0f, 10.0f), vec3(1.0f, 1.0f, 0.0f)},
    //    {vec3(16.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 1.0f)},
    //    {vec3(0.0f, 16.0f, 8.0f), vec3(0.0f, 1.0f, 1.0f)},
    //};

    // TODO(sean): froxel acceleration
    vec3 lighting_color = vec3(0.0f, 0.0f, 0.0f);

    for(int i = 0; i < light_count; i += 1) {
        vec3 position_difference = lights[i].position.xyz - in_position;
        float attenuation = 1.0f / sqrt(dot(position_difference, position_difference));

        vec3 light_direction = normalize(position_difference);

        float dotprod = dot(in_normal, light_direction);
        float shape = clamp((dotprod + 2.0f) / 2.0f, 0.0f, 1.0f);

        vec3 color = vec3(attenuation, attenuation, attenuation) * shape;

        lighting_color += color * lights[i].color.xyz;
    }

    out_color = vec4(lighting_color, 1.0f);
}
