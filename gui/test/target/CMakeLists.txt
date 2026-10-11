add_executable(psig_gui_target main.cpp)
target_compile_definitions(psig_gui_target PRIVATE UNICODE _UNICODE)
target_link_options(psig_gui_target PRIVATE /DYNAMICBASE:NO)
