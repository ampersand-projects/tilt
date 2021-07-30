#ifndef INCLUDE_TILT_BASE_LOG_H_
#define INCLUDE_TILT_BASE_LOG_H_

#include <iostream>

#define LOG(severity) \
    std::cerr << "[" << __FILE__ << ":" << __LINE__ << "] [" #severity "] "

#define ASSERT(EXPR, MSG) \
    if (!(EXPR)) { LOG(FATAL) << "Check failed: " #EXPR " " #MSG << std::endl; std::abort(); }

#define CHECK(EXPR) ASSERT(EXPR, "")

#endif // INCLUDE_TILT_BASE_LOG_H_