CMAKE_SOURCE_DIR=$1
CMAKE_CURRENT_BINARY_DIR=$2

clang++ -emit-llvm -S ${CMAKE_SOURCE_DIR}/src/pass/codegen/vinstr.cpp \
                   -I ${CMAKE_SOURCE_DIR}/include/ \
                   -o ${CMAKE_CURRENT_BINARY_DIR}/vinstr.ll

VINSTR_IR=$(cat ${CMAKE_CURRENT_BINARY_DIR}/vinstr.ll)

echo "#ifndef INCLUDE_VINSTR_STR_H_
#define INCLUDE_VINSTR_STR_H_
static const char* vinstr_str = R\"(
${VINSTR_IR}
)\";
#endif // INCLUDE_VINSTR_STR_H_
" > ${CMAKE_SOURCE_DIR}/include/tilt/pass/codegen/vinstr_str.h

rm ${CMAKE_CURRENT_BINARY_DIR}/vinstr.ll