add_executable(psig_target main.cpp)
target_compile_definitions(psig_target PRIVATE UNICODE _UNICODE)
target_link_options(psig_target PRIVATE /DYNAMICBASE:NO)
