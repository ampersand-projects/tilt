#ifndef INCLUDE_TILT_BASE_LOG_H_
#define INCLUDE_TILT_BASE_LOG_H_

#define LOG(severity) \
    std::cerr << "[" << __FILE__ << ":" << __LINE__ << "] [" #severity "] "

#define CHECK_VERBOSE(EXPR, MSG) \
    if (!(EXPR)) { LOG(FATAL) << "Check failed: " #EXPR " " #MSG << std::endl; std::abort(); }

#define CHECK(EXPR) CHECK_VERBOSE(EXPR, "")

#endif // INCLUDE_TILT_BASE_LOG_H_