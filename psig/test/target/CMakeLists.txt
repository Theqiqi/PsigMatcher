add_executable(psig_target main.cpp)
target_compile_definitions(psig_target PRIVATE UNICODE _UNICODE)
# 关闭 ASLR，便于跨进程测试时地址稳定
target_link_options(psig_target PRIVATE /DYNAMICBASE:NO)
