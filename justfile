default: build

configure:
    cmake -S . -B build -G Ninja

build: configure
    cmake --build build

run: build
    ./build/projctl

clean:
    rm -rf build
