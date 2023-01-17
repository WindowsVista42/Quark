#pragma once

// This file is only meant to be included inside of quark_platform.hpp
// quark_platform.hpp is included so LSP works
#include "../quark_platform.hpp"
using namespace quark;

//
// Custom Alignment
//

  inline u8* arena_push_with_alignment(Arena* arena, usize size, usize alignment) {
    usize new_size = align_forward(arena->position + size, alignment);
  
    u8* ptr = arena->ptr + arena->position;
    arena->position = new_size;
  
    // lazy
    while(arena->position > arena->commit_size) {
      os_commit_mem(arena->ptr + arena->commit_size, arena->commit_size);
      arena->commit_size *= 2;
    }
  
    return ptr;
  }

  inline u8* arena_push_zero_with_alignment(Arena* arena, usize size, usize alignment) {
    u8* ptr = arena_push_with_alignment(arena, size, alignment);
    zero_mem(ptr, size);
    return ptr;
  }

  inline u8* arena_copy_with_alignment(Arena* arena, void* src, usize size, usize alignment) {
    u8* ptr = arena_push_with_alignment(arena, size, alignment);
    copy_mem(ptr, src, size);
    return ptr;
  }

  #define arena_push_array_with_alignment(arena, type, count) (type*)arena_push_with_alignment((arena), (count) * sizeof(type))
  #define arena_push_array_zero_with_alignment(arena, type, count) (type*)arena_push_zero_with_alignment((arena), (count) * sizeof(type))

  #define arena_push_struct_with_alignment(arena, type) (type*)arena_push_with_alignment((arena), sizeof(type))
  #define arena_push_struct_zero_with_alignment(arena, type) (type*)arena_push_zero_with_alignment((arena), sizeof(type))

  #define arena_copy_array_with_alignment(arena, src, type, count) (type*)arena_copy_with_alignment((arena), (src), sizeof(type) * (count))

//
// Aligned
//

  inline u8* arena_push(Arena* arena, usize size) {
    return arena_push_with_alignment(arena, size, PTR_ALIGNMENT);
  }

  inline u8* arena_push_zero(Arena* arena, usize size) {
    return arena_push_zero_with_alignment(arena, size, PTR_ALIGNMENT);
  }

  inline u8* arena_copy(Arena* arena, void* src, usize size) {
    return arena_copy_with_alignment(arena, src, size, PTR_ALIGNMENT);
  }

  #define arena_push_array(arena, type, count) (type*)arena_push((arena), (count) * sizeof(type))
  #define arena_push_array_zero(arena, type, count) (type*)arena_push_zero((arena), (count) * sizeof(type))

  #define arena_push_struct(arena, type) (type*)arena_push((arena), sizeof(type))
  #define arena_push_struct_zero(arena, type) (type*)arena_push_zero((arena), sizeof(type))

  #define arena_copy_array(arena, src, type, count) (type*)arena_copy((arena), (src), sizeof(type) * (count))

//
// Helpers
//
  
  inline void arena_pop(Arena* arena, usize size) {
    arena->position -= size;
    arena->position = align_forward(arena->position, PTR_ALIGNMENT);
  }
  
  inline usize arena_get_position(Arena* arena) {
    return arena->position;
  }

  inline void arena_set_position_with_alignment(Arena* arena, usize new_position, usize alignment) {
    arena->position = new_position;
    arena->position = align_forward(arena->position, alignment);
  }
  
  inline void arena_set_position(Arena* arena, usize new_position) {
    arena_set_position_with_alignment(arena, new_position, PTR_ALIGNMENT);
  }
  
  inline void arena_clear(Arena* arena) {
    arena->position = 0;
  }
  
  inline void arena_clear_zero(Arena* arena) {
    zero_mem(arena->ptr, arena->position);
    arena->position = 0;
  }
  
  inline void arena_reset(Arena* arena) {
    os_decommit_mem(arena->ptr + 2 * MB, arena->position - 2 * MB);
  
    arena->position = 0;
    arena->commit_size = 2 * MB;
    zero_mem(arena->ptr, arena->commit_size);
  }

//
// Scratch API
//

  #define end_scratch(stack) end_temp_stack(stack)
