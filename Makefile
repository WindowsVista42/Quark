export CC = clang
export CXX = clang++

d:
	python3 compile_shaders.py
	ninja -C build/debug -f build.ninja
	cp build/debug/compile_commands.json .
	./build/debug/run

dr:
	python3 compile_shaders.py
	ninja -C build/debug -f build.ninja
	cp build/debug/compile_commands.json .

r:
	python3 compile_shaders.py
	ninja -C build/release -f build.ninja
	cp build/debug/compile_commands.json .
	./build/release/run

rr:
	python3 compile_shaders.py
	ninja -C build/release -f build.ninja
	cp build/debug/compile_commands.json .

w:
	python3 compile_shaders.py
	ninja -C build/release_with_debug_info -f build.ninja
	cp build/debug/compile_commands.json .
	./build/release_with_debug_info/run

wr:
	python3 compile_shaders.py
	ninja -C build/release_with_debug_info -f build.ninja
	cp build/debug/compile_commands.json .

b:
	rm -r build || true
	cmake -B build/debug -GNinja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ .
	cmake -B build/release -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ .
	cmake -B build/release_with_debug_info -GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo .
	cp build/debug/compile_commands.json .

f:
	clang-format src/*.cpp -i || true
	clang-format src/*.hpp -i || true
