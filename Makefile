#export CC = clang
#export CXX = clang++
export BIN = idler

d:
	python3 compile_shaders.py
	python3 build.py compile_run debug $(BIN)

dr:
	python3 compile_shaders.py
	python3 build.py compile debug $(BIN)

r:
	python3 compile_shaders.py
	python3 build.py compile_run release $(BIN)

rr:
	python3 compile_shaders.py
	python3 build.py compile release $(BIN)

w:
	python3 compile_shaders.py
	python3 build.py compile_run release_with_debug_info $(BIN)

wr:
	python3 compile_shaders.py
	python3 build.py compile release_with_debug_info $(BIN)

b:
	python3 build.py setup

f:
	clang-format src/*.cpp -i || true
	clang-format src/*.hpp -i || true
