cmake_minimum_required(VERSION 3.30)
add_library(mo2_notes SHARED)
set_property(TARGET mo2_notes PROPERTY CXX_STANDARD 20)
set(CMAKE_GENERATOR_PLATFORM x64)

target_include_directories(
        mo2_notes
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
#        PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include
)

if (MSVC)
    target_compile_options(
            mo2_notes
            PRIVATE
            /bigobj # increase number of sections in .obj file
            /W4     # level 4 warnings
            /WX     # treat all compiler warnings as errors
            /wd4201 # nonstandard extension used: nameless struct/union
            /wd4458 # declaration of 'identifier' hides class member
    )
endif ()

#target_link_libraries(mo2_notes PRIVATE pugixml nlohmann_json::nlohmann_json)
mo2_configure_plugin(
        mo2_notes
        WARNINGS OFF
        PRIVATE_DEPENDS
            boost uibase
)
mo2_install_target(mo2_notes)