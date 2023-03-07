#define QUARK_ENGINE_IMPLEMENTATION
#include "quark_engine.hpp"

#ifdef _WIN64
#include <windows.h>
#include <dbghelp.h>
#endif

#include <lz4.h>

namespace quark {
//
// Structs
//

  struct FileBuffer {
    Arena* arena;
    u8* start;
    usize size;
    usize read_pos;
  };

//
// Functions
//

  FileBuffer create_fileb(Arena* arena) {
    return FileBuffer {
      .arena = arena,
      .start = arena_push(arena, 0),
      .size = 0,
      .read_pos = 0,
    };
  }

  void write_fileb(FileBuffer* buffer, void* src, u32 element_size, u32 element_count) {
    arena_copy(buffer->arena, src, element_size * element_count);

    buffer->size += element_size * element_count;
    buffer->size = align_forward(buffer->size, 8);
  }

  void read_fileb(FileBuffer* buffer, void* dst, u32 element_size, u32 element_count) {
    copy_mem(dst, buffer->start + buffer->read_pos, element_size * element_count);

    buffer->read_pos += element_size * element_count;
    buffer->read_pos = align_forward(buffer->read_pos, 8);
  }

  void write_fileb_comp(FileBuffer* buffer, void* src, u32 element_size, u32 element_count) {
  }

  void read_fileb_comp(FileBuffer* buffer, void* dst, u32 byte_size) {
  }

  #pragma comment(lib, "dbghelp.lib")
  i32 find_static_section(const char* module_name, usize* static_size, void** static_ptr) {
    #ifdef DEBUG
    log_message("Loading .static section for " + module_name);
    #endif

    HMODULE hMod = GetModuleHandleA(module_name);
    if (hMod) {
      PIMAGE_NT_HEADERS64 NtHeader = ImageNtHeader(hMod);
      ULONGLONG ptr = NtHeader->OptionalHeader.ImageBase + NtHeader->OptionalHeader.SizeOfHeaders;
      WORD NumSections = NtHeader->FileHeader.NumberOfSections;
      PIMAGE_SECTION_HEADER Section = IMAGE_FIRST_SECTION(NtHeader);
      for (WORD i = 0; i < NumSections; i++) {
        if(strcmp((char*)Section->Name, ".static") == 0) {
          #ifdef DEBUG
          log_message(".static section found with size: " + (u32)Section->SizeOfRawData);
          #endif
          *static_size = Section->SizeOfRawData;
          *static_ptr = (void*)(NtHeader->OptionalHeader.ImageBase + Section->VirtualAddress);
          return 0;
        }
        Section++;
      }
    }
    return 1;
  }

  struct StaticSection {
    std::string name;
    usize size;
    void* ptr;
  };

  std::vector<StaticSection> static_sections;

  void add_plugin_name(const char* name) {
    static_sections.push_back({
      .name = name,
      .size = 0,
      .ptr = 0,
    });
  }

  void save_snapshot(const char* file) {
    for_every(i, static_sections.size()) {
      StaticSection* section = &static_sections[i];
      if(section->ptr == 0) {
        find_static_section(section->name.c_str(), &section->size, &section->ptr);
      }
    }

    Timestamp t0 = get_timestamp();
    defer({
      Timestamp t1 = get_timestamp();
      f64 delta_time = get_timestamp_difference(t0, t1);
      log_message("Saving snapshot took " + (f32)delta_time * 1000.0f + "ms");
    });

    File* f = open_file_panic_with_error(file, "wb", "Failed to open snapshot file for saving!\n");
    defer({
      close_file(f);
      #ifdef DEBUG
      log_message("Saved file!");
      #endif
    });

    EcsContext* ctx = get_resource(EcsContext);

    Arena* arena = get_arena();
    defer(free_arena(arena));

    FileBuffer b = create_fileb(arena);

    for_every(i, static_sections.size()) {
      write_fileb(&b, static_sections[i].ptr, 1, static_sections[i].size);
    }

    write_fileb(&b, &ctx->ecs_entity_head, sizeof(u32), 1);
    write_fileb(&b, &ctx->ecs_entity_tail, sizeof(u32), 1);
    write_fileb(&b, &ctx->ecs_empty_head, sizeof(u32), 1);
    for_every(i, ctx->ecs_table_count) {
      write_fileb(&b, ctx->ecs_bool_table[i], sizeof(u32), ECS_MAX_STORAGE / 32);
      write_fileb(&b, ctx->ecs_comp_table[i], ctx->ecs_comp_sizes[i], ECS_MAX_STORAGE);
    }

    u8* ptr = arena_push(arena, 8 * MB);
    i32 compress_size = LZ4_compress_default((const char*)b.start, (char*)ptr, b.size, 8 * MB);

    file_write(f, ptr, compress_size);
  }

  void load_snapshot(const char* file) {
    for_every(i, static_sections.size()) {
      StaticSection* section = &static_sections[i];
      if(section->ptr == 0) {
        print("Found section for: " + section->name.c_str());
        find_static_section(section->name.c_str(), &section->size, &section->ptr);
      }
    }

    Timestamp t0 = get_timestamp();
    defer({
      Timestamp t1 = get_timestamp();
      f64 delta_time = get_timestamp_difference(t0, t1);
      log_message("Loading snapshot took " + (f32)delta_time * 1000.0f + "ms");
    });

    Arena* arena = get_arena();
    defer(free_arena(arena));

    File* f = open_file_panic_with_error(file, "rb", "Failed to open snapshot state file for loading!\n");
    defer({
      close_file(f);
      #ifdef DEBUG
      log_message("Loaded file!");
      #endif
    });

    EcsContext* ctx = get_resource(EcsContext);

    FileBuffer b = create_fileb(arena);

    usize fsize = file_size(f);

    u8* ptr = arena_push(arena, 8 * MB);
    file_read(f, ptr, fsize);

    b.start = arena_push(arena, 64 * MB);
    b.size = LZ4_decompress_safe((const char*)ptr, (char*)b.start, fsize, 64 * MB);

    Timestamp s0 = get_timestamp();

    for_every(i, static_sections.size()) {
      read_fileb(&b, static_sections[i].ptr, 1, static_sections[i].size);
    }

    read_fileb(&b, &ctx->ecs_entity_head, sizeof(u32), 1);
    read_fileb(&b, &ctx->ecs_entity_tail, sizeof(u32), 1);
    read_fileb(&b, &ctx->ecs_empty_head, sizeof(u32), 1);
    for_every(i, ctx->ecs_table_count) {
      read_fileb(&b, ctx->ecs_bool_table[i], sizeof(u32), ECS_MAX_STORAGE / 32);
      read_fileb(&b, ctx->ecs_comp_table[i], ctx->ecs_comp_sizes[i], ECS_MAX_STORAGE);
    }
    Timestamp s1 = get_timestamp();
    #ifdef DEBUG
    log_message("Time to read_fileb: " + (f32)get_timestamp_difference(s0, s1) * 1000.0f + "ms, " + (f32)b.size / (f32)(1 * MB) +"mb");
    #endif

    // fread(&ctx->ecs_entity_head, sizeof(u32), 1, f);
    // fread(&ctx->ecs_entity_tail, sizeof(u32), 1, f);
    // for_every(i, ctx->ecs_table_count) {
    //   fread(ctx->ecs_bool_table[i], sizeof(u32), ECS_MAX_STORAGE / 32, f);
    //   fread(ctx->ecs_comp_table[i], ctx->ecs_comp_sizes[i], ECS_MAX_STORAGE, f);
    // }
  }

};
