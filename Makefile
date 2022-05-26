export CC = clang
export CXX = clang++
export BIN = idler

d:
	python3 compile_shaders.py
	ninja -C build/debug -f build.ninja
	cp build/debug/compile_commands.json .
	./build/debug/$(BIN)

dr:
	python3 compile_shaders.py
	ninja -C build/debug -f build.ninja
	cp build/debug/compile_commands.json .

r:
	python3 compile_shaders.py
	ninja -C build/release -f build.ninja
	cp build/debug/compile_commands.json .
	./build/release/$(BIN)

rr:
	python3 compile_shaders.py
	ninja -C build/release -f build.ninja
	cp build/debug/compile_commands.json .

w:
	python3 compile_shaders.py
	ninja -C build/release_with_debug_info -f build.ninja
	cp build/debug/compile_commands.json .
	./build/release_with_debug_info/$(BIN)

wr:
	python3 compile_shaders.py
	ninja -C build/release_with_debug_info -f build.ninja
	cp build/debug/compile_commands.json .

b:
	cmake -B build/debug -GNinja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=$(CC) -DCMAKE_CXX_COMPILER=$(CXX) .
	cmake -B build/release -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=$(CC) -DCMAKE_CXX_COMPILER=$(CXX) .
	cmake -B build/release_with_debug_info -GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_C_COMPILER=$(CC) -DCMAKE_CXX_COMPILER=$(CXX) .
	cp build/debug/compile_commands.json .

f:
	clang-format src/*.cpp -i || true
	clang-format src/*.hpp -i || true
