

default:
	mkdir -p build && cd build && cmake .. && make -j

r:
	./build/poker

br: default r
