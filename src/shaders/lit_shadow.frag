#version 460

layout (location = 0) in vec3 in_position;

layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texture;
layout (location = 3) flat in uint in_texture_index;
//layout (location = 4) flat in uint in_base_instance;
layout (location = 4) in vec4 in_sun_position;

struct RawLight {
  vec4 position; // w is falloff distance
  vec4 color;
};

layout (set = 0, binding = 0) uniform RenderConstants {
  RawLight lights[1024];
  uint light_count;
  uint _pad0;
  uint _pad1;
  uint _pad2;
  vec4 camera_direction;
  vec4 camera_position;
  float time;
  uint _pad3;
  uint _pad4;
  uint _pad5;
  mat4 sun_view_projection;
};

layout (location = 0) out vec4 out_color;

vec3 aces(vec3 x) {
  const float a = 2.51;
  const float b = 0.03;
  const float c = 2.43;
  const float d = 0.59;
  const float e = 0.14;
  return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main() {
  const vec3 normal_color = (in_normal + 1.0f) / 2.0f;
  //const vec3 normal_color = vec3(in_texture, 1.0f);
  //vec4 color;

  //out_color = vec4(normal_color, 1.0f);;

  //const Light light[3] = {
  //  {vec3(0.0f, 0.0f, 10.0f), vec3(1.0f, 1.0f, 0.0f)},
  //  {vec3(16.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 1.0f)},
  //  {vec3(0.0f, 16.0f, 8.0f), vec3(0.0f, 1.0f, 1.0f)},
  //};

  // TODO(sean): froxel acceleration
  vec3 lighting_color = vec3(0.0f, 0.0f, 0.0f);

  vec3 view_dir = normalize(camera_position.xyz - in_position);

  for(int i = 0; i < light_count; i += 1) {
    vec3 position_difference = lights[i].position.xyz - in_position;
    float dist = length(position_difference);
    float d = lights[i].position.w;
    float x = clamp((d - dist) / d, 0.0f, 1.0f);
    //float attenuation = smoothstep(0.0f, 1.0f, x);

    const float PI = 3.14159286;

    float attenuation = pow(abs(x), 4.0f);

    //dist = clamp((d - dist) / d, 0.0f, 1.0f);
    //float attenuation = pow(min(cos(PI * dist / 2.0), 1.0 - abs(dist)), 2.0);

    vec3 light_direction = normalize(position_difference);

    float dotprod = dot(in_normal, light_direction);
    float shape = (dotprod + 2.0f) / 2.0f;

    vec3 color = vec3(attenuation) * shape;

    lighting_color += color * lights[i].color.xyz;
  }

  //lighting_color.b *= 2.0f;

  //lighting_color *= 2.0f;

  //vec3 pos = in_position;
  //pos.z += time * 0.1f;

  //pos *= 0.5f;

  //pos = snoise(pos) * pos;

  //float fac = snoise(pos * 2.0f);
  //fac += snoise(pos * 4.0f);
  //fac += snoise(pos * 16.0f);
  //if (fac > 0.7f) {
  //  lighting_color *= 2.0f;
  //}

  ////vec3 pos = in_position * 0.125;
  ////pos.z -= time * 0.1f;
  ////pos.x -= time * 0.5f;
  ////pos.y -= time * 0.2f;

  ////const float off = 1.0f;
  ////float fac = snoise(vec3(snoise(pos)));
  ////fac += snoise(vec3(snoise(pos - (off * in_tangent))));
  ////fac += snoise(vec3(snoise(pos + (off * in_tangent))));
  ////fac += snoise(vec3(snoise(pos - (off * in_bitangent))));
  ////fac += snoise(vec3(snoise(pos + (off * in_bitangent))));

  ////pos *= 4.0f;

  ////fac += snoise(vec3(snoise(pos)));
  ////fac += snoise(vec3(snoise(pos - (off * in_tangent))));
  ////fac += snoise(vec3(snoise(pos + (off * in_tangent))));
  ////fac += snoise(vec3(snoise(pos - (off * in_bitangent))));
  ////fac += snoise(vec3(snoise(pos + (off * in_bitangent))));

  ////pos *= 4.0f;

  //fac += snoise(vec3(snoise(pos)));
  //fac += snoise(vec3(snoise(pos - (0.01f * in_tangent))));
  //fac += snoise(vec3(snoise(pos + (0.01f * in_tangent))));
  //fac += snoise(vec3(snoise(pos - (0.01f * in_bitangent))));
  //fac += snoise(vec3(snoise(pos + (0.01f * in_bitangent))));

  //fac /= 10.0f;

  ////fac /= 10.0f;

  ////fac += 1.0f;
  ////fac /= 2.0f;

  ////lighting_color *= fac;//lighting_color * fac;

  //if(fac > 0.4f) {
  //  fac *= 4.0f;
  //} else {
  //  fac = 1.0f;
  //}

  // Another Metalic?
  //vec3 pixel_dir = normalize(camera_position.xyz - in_position);
  //lighting_color += lighting_color * (dot(pixel_dir, in_normal));

  //lighting_color *= fac;

  // Metalic
  vec3 sun_pos = in_sun_position.xyz / in_sun_position.w;
  vec3 ad = vec3(0.0f, 0.0f, 0.0f);
  if(sun_pos.x > 0.0f) {
    ad = vec3(1.0f, 1.0f, 1.0f);
  }

  vec3 tonemapped = aces(lighting_color + ad);

  // Toon?
  //const float factor = 64.0f;
  //vec3 large = tonemapped;
  //large.x = float(int(large.x));
  //large.y = float(int(large.y));
  //large.z = float(int(large.z));
  //large /= factor;
  //tonemapped = large;

  out_color = vec4(tonemapped, 1.0f);
}
