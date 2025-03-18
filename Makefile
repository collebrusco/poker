

default:
	mkdir -p build && cd build && cmake .. && make -j

b:
	mkdir -p build && cd build && make -j

r:
	./build/poker

br: b r

bbr: default r
