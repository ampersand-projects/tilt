# TiLT: A Temporal Query Compiler
TiLT is a query compiler and execution engine for temporal stream processing applications.

## Building TiLT from source

### Prerequisites
 1. CMake 3.13.4
 2. LLVM 15
 3. Clang++ 15

### Build and install LLVM and Clang
Download and unpack [llvm-project-15.0.7.tar.xz](https://github.com/llvm/llvm-project/releases/download/llvmorg-15.0.7/llvm-project-15.0.7.src.tar.xz)

    cd llvm-project-15.0.7
    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release \
          -DLLVM_ENABLE_RTTI=ON \
          -DLLVM_TARGETS_TO_BUILD="X86" \
          -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra" \
          -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi" \
          -DLLVM_ENABLE_ZLIB=OFF \
          -DLLVM_ENABLE_ZSTD=OFF \
          -DLLVM_ENABLE_TERMINFO=OFF \
          -DLLVM_BUILD_LLVM_DYLIB=ON \
          -DLLVM_LINK_LLVM_DYLIB=ON \
          -DCMAKE_INSTALL_PREFIX=<install_path> ../llvm
    cmake --build .
    cmake --build . --target install

### Build TiLT
Clone TiLT repository along with the submodules

    git clone https://github.com/ampersand-projects/tilt.git --recursive
    mkdir build
    cd build
    cmake -DLLVM_DIR=<install_path>/lib/cmake/llvm ..
    cmake --build .

## Building PyTiLT from source

    export LLVM_INSTALL_PATH=<install_path>
    python3 setup.py install
