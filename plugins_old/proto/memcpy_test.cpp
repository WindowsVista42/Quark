#include <iostream>
#include <array>
#include <chrono>

struct Data {
  int a;
  int b;
  int c;
  int d;
  int e;
  int f;
  int g;
  int h;
};

#define GROUPS 8
#define ENTRIES 200
#define TESTS 16000

static std::array<std::array<Data, ENTRIES>, GROUPS> data_dst;

static std::array<std::array<Data, ENTRIES>, GROUPS> data_flat;
static std::array<std::pair<Data, int>, ENTRIES * GROUPS> data_offsets;

void copy_flat() {
  std::array<int, GROUPS> lengths = {0,0,0,0,0,0,0,0};
  for (int i = 0; i < GROUPS; i += 1) {
    for (int j = 0; j < ENTRIES; j += 1) {
      data_dst[i][lengths[i]] = data_flat[i][j];
      lengths[i] += 1;
    }
  }
}

void copy_offsets() {
  std::array<int, GROUPS> lengths = {0,0,0,0,0,0,0,0};
  for (int i = 0; i < GROUPS * ENTRIES; i += 1) {
    auto [data, group] = data_offsets[i];
    data_dst[group][lengths[group]] = data;
    lengths[group] += 1;
  }
}

int main() {

  for (int i = 0; i < GROUPS; i += 1) {
    for (int j = 0; j < ENTRIES; j += 1) {
      Data data = {
        .a = rand(),
        .b = rand(),
        .c = rand(),
        .d = rand(),
        .e = rand(),
        .f = rand(),
        .g = rand(),
        .h = rand(),
      };

      data_flat[i][j] = data;
      data_offsets[(i * ENTRIES) + j] = std::make_pair(data, i);
    }
  }

  auto t0 = std::chrono::high_resolution_clock::now();
  for(int k = 0; k < TESTS; k += 1) {
    copy_flat();
  }
  auto t1 = std::chrono::high_resolution_clock::now();
  std::cout << (std::chrono::duration<double>(t1 - t0).count() / (double)(TESTS)) * 1000.0 << std::endl;

  printf("%d\n", data_dst[0][0].a);

  t0 = std::chrono::high_resolution_clock::now();
  for(int k = 0; k < TESTS; k += 1) {
    copy_offsets();
  }
  t1 = std::chrono::high_resolution_clock::now();
  std::cout << (std::chrono::duration<double>(t1 - t0).count() / (double)(TESTS)) * 1000.0 << std::endl;

  printf("%d\n", data_dst[0][0].a);
}
