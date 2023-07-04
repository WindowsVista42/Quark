#define QUARK_ENGINE_IMPLEMENTATION
#include "quark_engine.hpp"

#undef format

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"

  #define MINIAUDIO_IMPLEMENTATION
  #include "../../lib/miniaudio/miniaudio.h"

#pragma clang diagnostic pop

#define api_decl engine_api
#define var_decl engine_var

namespace quark {
//
// Resources
//

  declare_component(SoundData,
    ma_sound sound;
    Transform previous_transform;
  );

  declare_component(TemporarySound);

  define_component(SoundOptions);
  define_component(SoundData);
  define_component(TemporarySound);

  define_resource(SoundContext, {});
  define_resource(MainListener, {});

//
// Functions
//

  inline ma_engine* sound_engine() {
    return get_resource(SoundContext)->engine;
  }

  inline vec3 listener_position() {
    return get_resource(MainListener)->position;
  }

  inline quat listener_rotation() {
    return get_resource(MainListener)->rotation;
  }

  void init_sound_context() {
    update_component(SoundOptions);
    update_component(SoundData);
    update_component(TemporarySound);
  
    SoundContext* ctx = get_resource(SoundContext);
  
    ctx->engine = (ma_engine*)arena_push(global_arena(), sizeof(ma_engine));
    if(ma_result result = ma_engine_init(0, ctx->engine); result != MA_SUCCESS) {
      panic("Failed to init audio with error: " + (u32)result + "\n");
    }

    ma_engine_listener_set_enabled(sound_engine(), 0, true);
    ma_engine_listener_set_world_up(sound_engine(), 0, 0, 0, 1);
  }

  void attach_sound(EntityId id, const char* sound_path) {
    if(!has_component(id, Transform)) {
      panic("In attach_sound_to_entity(), entity does not have a Transform component!\n");
    }
  
    if(!has_component(id, SoundOptions)) {
      panic("In attach_sound_to_entity(), entity does not have a SoundOptions component!\n");
    }
  
    add_components(id, SoundData {});
    SoundData* data = get_component(id, SoundData);
    ma_sound* ref = &data->sound;

    if(ma_result result = ma_sound_init_from_file(sound_engine(), sound_path, MA_SOUND_FLAG_DECODE, 0, 0, ref); result != MA_SUCCESS) {
      panic("Failed to find init sound: " + sound_path + "\n");
    };

    if(ma_result result = ma_sound_start(ref); result != MA_SUCCESS) {
      panic("Failed to start sound: " + sound_path + "\n");
    }

    Transform* transform = get_component(id, Transform);
    data->previous_transform = *transform;
    update_sound_and_options(id, transform, get_component(id, SoundOptions));
  }

  EntityId spawn_sound(const char* sound_path, Transform transform, SoundOptions options, bool persist) {
    EntityId id = create_entity();

    add_components(id, transform, options);
    attach_sound(id, sound_path);

    if(!persist) {
      add_components(id, TemporarySound {});
    }

    return id;
  }

  ma_attenuation_model ma_attenuation_model_table[] = {
    ma_attenuation_model_none,
    ma_attenuation_model_inverse,
    ma_attenuation_model_linear,
    ma_attenuation_model_exponential,
  };

  void update_sound_and_options(EntityId id, Transform* transform, SoundOptions* options) {
    SoundData* data = get_component(id, SoundData);
    ma_sound* ref = &data->sound;

    ma_sound_set_looping(ref, (ma_bool32)options->loop);

    vec3 velocity = (data->previous_transform.position - transform->position) * delta();
    data->previous_transform.position = transform->position;

    ma_sound_set_position(ref, transform->position.x, transform->position.y, transform->position.z);
    ma_sound_set_velocity(ref, velocity.x, velocity.y, velocity.z);
    ma_sound_set_volume(ref, options->volume);
    ma_sound_set_pitch(ref, options->pitch);
    ma_sound_set_rolloff(ref, options->rolloff);
  
    ma_sound_set_attenuation_model(ref, ma_attenuation_model_table[(u32)options->attenuation_model]);
    ma_sound_set_min_gain(ref, options->min_gain);
    ma_sound_set_max_gain(ref, options->max_gain);
    ma_sound_set_min_distance(ref, options->min_distance);
    ma_sound_set_max_distance(ref, options->max_distance);

    vec3 direction = quat_forward(transform->rotation);
    ma_sound_set_direction(ref, direction.x, direction.y, direction.z);
    ma_sound_set_cone(ref, options->inner_cone_angle, options->outer_cone_angle, options->outer_cone_gain);

    options->playing = ma_sound_is_playing(ref);
  }

  void sync_sound_state() {
    MainListener* listener = get_resource(MainListener);
  
    vec3 forward = quat_forward(listener->rotation);
    vec3 up = quat_up(listener->rotation);

    vec3 velocity = (listener->previous_position - listener->position) * delta();
    listener->previous_position = listener->position;
  
    ma_engine_listener_set_position(sound_engine(), 0, listener->position.x, listener->position.y, listener->position.z);
    ma_engine_listener_set_velocity(sound_engine(), 0, velocity.x, velocity.y, velocity.z);
    ma_engine_listener_set_direction(sound_engine(), 0, forward.x, forward.y, forward.z);
    ma_engine_listener_set_world_up(sound_engine(), 0, up.x, up.y, up.z);

    for_archetype(Include<Transform, SoundOptions, SoundData, TemporarySound> {}, Exclude<> {},
    [&](EntityId id, Transform*, SoundOptions*, SoundData* data, TemporarySound*) {
      if(!ma_sound_is_playing(&data->sound)) {
        destroy_entity(id);
      }
    });
  }

  void purge_non_persistent_sounds() {
  }
};

#undef api_decl
#undef var_decl
