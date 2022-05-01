// INCLUDE: DEFAULT
// SHADER_TYPE: DEFAULT
// PUSH_CONSTANT: DEFAULT
// WORLD_DATA: DEFAULT

// SECTION: VERTEX

void main() {
  out_position = (rotate(in_position, world_rotation) * world_scale.xyz) + world_position.xyz;
  out_normal = rotate(in_normal, world_rotation);
  out_texture = in_texture;
  out_texture_index = floatBitsToUint(world_position.w);

  out_sun_position = sun_view_projection * vec4(out_position, 1.0f);
  gl_Position = world_view_projection * vec4(in_position, 1.0f);
}

// SECTION: FRAGMENT

vec3 aces(vec3 x) {
  const float a = 2.51;
  const float b = 0.03;
  const float c = 2.43;
  const float d = 0.59;
  const float e = 0.14;
  return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

vec3 toonify(vec3 color, const float layers) {
  vec3 large = pow(color, vec3(1.0 / 2.2)) * layers;
  large = vec3(ivec3(large));
  large /= layers;
  return pow(large, vec3(2.2));
}

vec3 diffuse_point_light(PointLightData light, vec3 pixel_pos, vec3 pixel_normal) {
  vec3 pos_diff = light.position - pixel_pos;
  vec3 light_dir = normalize(pos_diff);
  float distance = length(pos_diff);

  float x = clamp((light.falloff - distance) / light.falloff, 0.0f, 1.0f);
  float attenuation = pow(abs(x), 4.0f);
  float factor = dot(pixel_normal, light_dir);
  float shape_half = max(factor, 0.0f);
  float shape = mix(1.0f, shape_half, light.directionality);
  float brightness = attenuation * shape;

  return light.color * brightness;
}

// TODO(Sean): add position falloff to this
vec3 diffuse_directional_light(DirectionalLightData light, vec3 pixel_pos, vec3 pixel_normal) {
  vec3 pos_diff = light.position - pixel_pos;
  vec3 light_dir = light.direction;
  float distance = length(pos_diff);

  float x = clamp((light.falloff - distance) / light.falloff, 0.0f, 1.0f);
  float attenuation = pow(abs(x), 4.0f);
  float factor = -dot(pixel_normal, light_dir);
  float shape_half = max(factor, 0.0f);
  float shape = mix(1.0f, shape_half, light.directionality);
  float brightness = attenuation * shape;

  return light.color * brightness;
}

vec3 diffuse_sun_light(SunLightData light, vec3 pixel_normal) {
  float factor = -dot(pixel_normal, light.direction);
  float shape_half = max(factor, 0.0f);
  float shape = mix(1.0f, shape_half, light.directionality);
  float brightness = shape;

  return light.color * brightness;
}

vec3 shadow_directional(in sampler2D shadow_sampler, SunLightData light, vec4 projected_pos, vec3 pixel_normal) {
  vec3 proj_coords = projected_pos.xyz / projected_pos.w; // do this interpolated in the vertex shader?
  proj_coords.xy = proj_coords.xy * 0.5f + 0.5f;

  const float bias_max = 0.00005; // magic number 1
  const float bias_min = bias_max / 4.0f; // magic number 2
  float bias = max(bias_max * (1.0f - dot(pixel_normal, light.direction)), bias_min);
  float closest_depth = texture(shadow_sampler, proj_coords.xy).r;
  float current_depth = proj_coords.z;
  float shadow = 0.0;
  vec2 texel_size = 1.0 / vec2(128);
  float pcf_depth = texture(sun_shadow_sampler, proj_coords.xy + vec2(0, 0) * texel_size).r; 
  shadow += current_depth - bias > pcf_depth ? 1.0 : 0.0;
  //for(int x = -1; x <= 1; ++x) {
  //  for(int y = -1; y <= 1; ++y) {
  //    float pcf_depth = texture(sun_shadow_sampler, proj_coords.xy + vec2(x, y) * texel_size).r; 
  //    shadow += current_depth - bias > pcf_depth ? 1.0 : 0.0;
  //  }
  //}
  shadow /= 2.0;

  if(proj_coords.x >= 0.95f || proj_coords.x <= 0.05f) { shadow = 0.0f; }
  if(proj_coords.y >= 0.95f || proj_coords.y <= 0.05f) { shadow = 0.0f; }

  if(dot(pixel_normal, light.direction) > -0.05f) { shadow = 1.0f; }

  return light.color * (1.0f - shadow);
}

void main() {
  const vec3 view_dir = normalize(main_camera.pos - in_position);
  const vec3 color = vec3(1.0f, 1.0f, 1.0f); // sample texture map
  const vec3 ambient = vec3(0.0f);

  vec3 diffuse = vec3(0.0f);
  vec3 specular = vec3(0.0f);

  for(int i = 0; i < point_light_count; i += 1) {
    diffuse += diffuse_point_light(point_lights[i], in_position, in_normal);
  }

  for(int i = 0; i < directional_light_count; i += 1) {
    diffuse += diffuse_directional_light(directional_lights[i], in_position, in_normal);
  }

  vec3 sun = diffuse_sun_light(sun_light, in_normal);
  vec3 shadow = shadow_directional(sun_shadow_sampler, sun_light, in_sun_position, in_normal);

  vec3 lighting = (sun * shadow) + diffuse + specular;
  vec3 result = lighting * color;
  result = pow(result, vec3(2.2));
  vec3 tonemapped = aces(result);
  //tonemapped = toonify(tonemapped, 20.0f);

  out_color = vec4(tonemapped, 1.0f);
}
