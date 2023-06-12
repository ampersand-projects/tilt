CMAKE_SOURCE_DIR=$1
CMAKE_CURRENT_BINARY_DIR=$2

clang++ -emit-llvm -S ${CMAKE_SOURCE_DIR}/src/pass/codegen/vinstr.cpp \
                   -I ${CMAKE_SOURCE_DIR}/include/ \
                   -o ${CMAKE_CURRENT_BINARY_DIR}/vinstr.ll

VINSTR_IR=$(cat ${CMAKE_CURRENT_BINARY_DIR}/vinstr.ll)

echo "const char* vinstr_str = R\"(
${VINSTR_IR}
)\";
" > ${CMAKE_SOURCE_DIR}/src/pass/codegen/vinstr_str.cpp

rm ${CMAKE_CURRENT_BINARY_DIR}/vinstr.ll
