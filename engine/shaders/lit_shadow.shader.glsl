// INCLUDE: DEFAULT
// SHADER_TYPE: DEFAULT
// PUSH_CONSTANT: DEFAULT
// WORLD_DATA: DEFAULT
//
// Simplex 2D noise
//
vec3 permute(vec3 x) { return mod(((x*34.0)+1.0)*x, 289.0); }

float snoise(vec2 v){
  const vec4 C = vec4(0.211324865405187, 0.366025403784439,
           -0.577350269189626, 0.024390243902439);
  vec2 i  = floor(v + dot(v, C.yy) );
  vec2 x0 = v -   i + dot(i, C.xx);
  vec2 i1;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;
  i = mod(i, 289.0);
  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
  + i.x + vec3(0.0, i1.x, 1.0 ));
  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy),
    dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;
  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );
  vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}

//	Simplex 4D Noise 
//	by Ian McEwan, Ashima Arts
//
vec4 permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}
float permute(float x){return floor(mod(((x*34.0)+1.0)*x, 289.0));}
vec4 taylorInvSqrt(vec4 r){return 1.79284291400159 - 0.85373472095314 * r;}
float taylorInvSqrt(float r){return 1.79284291400159 - 0.85373472095314 * r;}

vec4 grad4(float j, vec4 ip){
  const vec4 ones = vec4(1.0, 1.0, 1.0, -1.0);
  vec4 p,s;

  p.xyz = floor( fract (vec3(j) * ip.xyz) * 7.0) * ip.z - 1.0;
  p.w = 1.5 - dot(abs(p.xyz), ones.xyz);
  s = vec4(lessThan(p, vec4(0.0)));
  p.xyz = p.xyz + (s.xyz*2.0 - 1.0) * s.www; 

  return p;
}

float snoise(vec4 v){
  const vec2  C = vec2( 0.138196601125010504,  // (5 - sqrt(5))/20  G4
                        0.309016994374947451); // (sqrt(5) - 1)/4   F4
// First corner
  vec4 i  = floor(v + dot(v, C.yyyy) );
  vec4 x0 = v -   i + dot(i, C.xxxx);

// Other corners

// Rank sorting originally contributed by Bill Licea-Kane, AMD (formerly ATI)
  vec4 i0;

  vec3 isX = step( x0.yzw, x0.xxx );
  vec3 isYZ = step( x0.zww, x0.yyz );
//  i0.x = dot( isX, vec3( 1.0 ) );
  i0.x = isX.x + isX.y + isX.z;
  i0.yzw = 1.0 - isX;

//  i0.y += dot( isYZ.xy, vec2( 1.0 ) );
  i0.y += isYZ.x + isYZ.y;
  i0.zw += 1.0 - isYZ.xy;

  i0.z += isYZ.z;
  i0.w += 1.0 - isYZ.z;

  // i0 now contains the unique values 0,1,2,3 in each channel
  vec4 i3 = clamp( i0, 0.0, 1.0 );
  vec4 i2 = clamp( i0-1.0, 0.0, 1.0 );
  vec4 i1 = clamp( i0-2.0, 0.0, 1.0 );

  //  x0 = x0 - 0.0 + 0.0 * C 
  vec4 x1 = x0 - i1 + 1.0 * C.xxxx;
  vec4 x2 = x0 - i2 + 2.0 * C.xxxx;
  vec4 x3 = x0 - i3 + 3.0 * C.xxxx;
  vec4 x4 = x0 - 1.0 + 4.0 * C.xxxx;

// Permutations
  i = mod(i, 289.0); 
  float j0 = permute( permute( permute( permute(i.w) + i.z) + i.y) + i.x);
  vec4 j1 = permute( permute( permute( permute (
             i.w + vec4(i1.w, i2.w, i3.w, 1.0 ))
           + i.z + vec4(i1.z, i2.z, i3.z, 1.0 ))
           + i.y + vec4(i1.y, i2.y, i3.y, 1.0 ))
           + i.x + vec4(i1.x, i2.x, i3.x, 1.0 ));
// Gradients
// ( 7*7*6 points uniformly over a cube, mapped onto a 4-octahedron.)
// 7*7*6 = 294, which is close to the ring size 17*17 = 289.

  vec4 ip = vec4(1.0/294.0, 1.0/49.0, 1.0/7.0, 0.0) ;

  vec4 p0 = grad4(j0,   ip);
  vec4 p1 = grad4(j1.x, ip);
  vec4 p2 = grad4(j1.y, ip);
  vec4 p3 = grad4(j1.z, ip);
  vec4 p4 = grad4(j1.w, ip);

// Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;
  p4 *= taylorInvSqrt(dot(p4,p4));

// Mix contributions from the five corners
  vec3 m0 = max(0.6 - vec3(dot(x0,x0), dot(x1,x1), dot(x2,x2)), 0.0);
  vec2 m1 = max(0.6 - vec2(dot(x3,x3), dot(x4,x4)            ), 0.0);
  m0 = m0 * m0;
  m1 = m1 * m1;
  return 49.0 * ( dot(m0*m0, vec3( dot( p0, x0 ), dot( p1, x1 ), dot( p2, x2 )))
               + dot(m1*m1, vec2( dot( p3, x3 ), dot( p4, x4 ) ) ) ) ;
}

// SECTION: VERTEX

void main() {
  WORLD_POSITION = rotate(VERTEX_POSITION * MODEL_SCALE.xyz, MODEL_ROTATION) + MODEL_POSITION.xyz;
  WORLD_NORMAL = rotate(VERTEX_NORMAL, MODEL_ROTATION);
  WORLD_UV = VERTEX_UV;
  //WORLD_TEX_ID = 0;

  SUN_POSITION = sun_view_projection * vec4(WORLD_POSITION, 1.0f);
  POSITION = main_view_projection * vec4(WORLD_POSITION, 1.0f);
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
  vec3 pos_diff = vec3(ivec3((light.position - pixel_pos + 0.0001) * 8)) / 8.0f;
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
  vec3 pos_diff = vec3(ivec3((light.position - pixel_pos + 0.0001) * 8)) / 8.0f;
  //vec3 pos_diff = light.position - pixel_pos;
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
  float factor = float(int(-dot(pixel_normal, light.direction) * 10)) / 10.0f;
  float shape_half = max(factor, 0.0f);
  float shape = mix(1.0f, shape_half, light.directionality);
  float brightness = shape;

  vec3 r = 2.0f * pixel_normal * dot(pixel_normal, -light.direction) - light.direction;
  vec3 d = normalize(WORLD_POSITION - main_camera.pos);
  float z = pow(dot(d, r), 9.0f);
  float fr = pow(1.0f - pow((1.0f + dot(d, light.direction)) / 2.0f, 0.5f), 5.0f);
  float h = z * (0.05 + 0.95 * fr);

  return light.color * brightness + clamp(h, 0.0f, 1.0f);
}

vec3 shadow_directional(in sampler2D shadow_sampler, SunLightData light, vec4 projected_pos, vec3 pixel_normal) {
  vec3 proj_coords = projected_pos.xyz / projected_pos.w; // do this interpolated in the vertex shader?
  proj_coords.xy = proj_coords.xy * 0.5f + 0.5f;

  const float bias_max = 0.00008; // magic number 1
  const float bias_min = bias_max / 4.0f; // magic number 2
  float bias = max(bias_max * (1.0f - dot(pixel_normal, light.direction)), bias_min);
  float closest_depth = texture(shadow_sampler, proj_coords.xy).r;
  float current_depth = proj_coords.z;
  float shadow = 0.0;
  //float m = min(800.0f, 512.0f * ( snoise( vec4(WORLD_POSITION.xyz, TT)) + 1.0f))
  //vec2 texel_size = 1.0 / vec2(max(300.0f, m));
  vec2 texel_size = 1.0 / vec2(512.0f);
  //float pcf_depth = texture(sun_shadow_sampler, proj_coords.xy + vec2(0, 0) * texel_size).r; 
  //shadow += current_depth - bias > pcf_depth ? 1.0 : 0.0;
  for(int x = -1; x <= 1; ++x) {
    for(int y = -1; y <= 1; ++y) {
      float pcf_depth = texture(sun_shadow_sampler, proj_coords.xy + vec2(x, y) * texel_size).r;
      shadow += current_depth - bias > pcf_depth ? 1.0 : 0.0;
    }
  }
  shadow /= 10.0;

  if(proj_coords.x >= 0.95f || proj_coords.x <= 0.05f) { shadow = 0.0f; }
  if(proj_coords.y >= 0.95f || proj_coords.y <= 0.05f) { shadow = 0.0f; }

  if(dot(pixel_normal, light.direction) > -0.05f) { shadow = 1.0f; }

  return light.color * (1.0f - shadow);
}

void main() {
  const vec3 view_dir = normalize(main_camera.pos - WORLD_POSITION);
  const vec3 color = texture(TEXTURES[TEXTURE_INDEX], WORLD_UV).xyz;
  const vec3 ambient = vec3(0.0f);

  vec3 diffuse = vec3(0.0f);
  vec3 specular = vec3(0.0f);

  for(int i = 0; i < point_light_count; i += 1) {
    diffuse += diffuse_point_light(point_lights[i], WORLD_POSITION, WORLD_NORMAL);
  }

  for(int i = 0; i < directional_light_count; i += 1) {
    diffuse += diffuse_directional_light(directional_lights[i], WORLD_POSITION, WORLD_NORMAL);
  }

  vec3 a = vec3(ivec3((WORLD_POSITION * 0.5) * 16 + 0.0001)) / 16.0;

  vec3 sun = diffuse_sun_light(sun_light, WORLD_NORMAL);
  vec3 shadow = shadow_directional(sun_shadow_sampler, sun_light, SUN_POSITION, WORLD_NORMAL);

  vec3 lighting = (sun * shadow) + diffuse + specular;
  vec3 result = 2.0f * lighting * color;
  //result = pow(result, vec3(2.2));
  vec3 ran = vec3(snoise(vec4(a, TT * 0.125)) * 0.01) + vec3(snoise(vec4(a, TT * 0.125) * 4.0f) * 0.01);
  vec3 tonemapped = aces(result + ran);
  tonemapped = toonify(tonemapped, 20.0f);

  COLOR = vec4(tonemapped, 1.0f);
}
