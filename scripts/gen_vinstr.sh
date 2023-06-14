CMAKE_CXX_COMPILER=$1
CMAKE_CURRENT_SOURCE_DIR=$2
CMAKE_CURRENT_BINARY_DIR=$3

${CMAKE_CXX_COMPILER} -emit-llvm -S ${CMAKE_CURRENT_SOURCE_DIR}/pass/codegen/vinstr.cpp \
                   -I ${CMAKE_CURRENT_SOURCE_DIR}/../include/ \
                   -o ${CMAKE_CURRENT_BINARY_DIR}/vinstr.ll

VINSTR_IR=$(cat ${CMAKE_CURRENT_BINARY_DIR}/vinstr.ll)

echo "const char* vinstr_str = R\"(
${VINSTR_IR}
)\";
" > ${CMAKE_CURRENT_BINARY_DIR}/vinstr_str.cpp
