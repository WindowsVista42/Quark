#define QUARK_ENGINE_IMPLEMENTATION
#include "quark_engine.hpp"

#undef format

#define MINIAUDIO_IMPLEMENTATION
#include "../../lib/miniaudio/miniaudio.h"

namespace quark {
//
// Audio API
//

  define_resource(SoundContext, {});

  void init_sound_context() {
    SoundContext* ctx = get_resource(SoundContext);
  
    ctx->engine = (ma_engine*)arena_push(global_arena(), sizeof(ma_engine));
    if(ma_result result = ma_engine_init(0, ctx->engine); result != MA_SUCCESS) {
      panic("Failed to init audio with error: " + (u32)result + "\n");
    }
  }

  void play_sound(const char* sound_name, vec2 position) {
    SoundContext* ctx = get_resource(SoundContext);
  
    ma_engine_play_sound(ctx->engine, sound_name, 0);
  }
};
