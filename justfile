default: build

configure:
    cmake -S . -B build -G Ninja

build: configure
    cmake --build build

run *args: build
    ./build/projctl {{args}}

clean:
    rm -rf build
