cmake_minimum_required(VERSION 3.30)
include(FetchContent)
add_library(mo2_notes SHARED)
set_property(TARGET mo2_notes PROPERTY CXX_STANDARD 20)
set(CMAKE_GENERATOR_PLATFORM x64)

set(CMAKE_AUTORCC ON) # Will make the compiler read QRC file and skip the JS stuff.


FetchContent_Declare(
        QMarkdownTextEdit
        GIT_REPOSITORY https://github.com/pbek/QMarkdownTextEdit.git
        GIT_TAG main
)
FetchContent_MakeAvailable(QMarkdownTextEdit)

target_include_directories(
        mo2_notes
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
        PRIVATE ${qmarkdowntextedit_SOURCE_DIR}
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

target_link_libraries(mo2_notes PRIVATE qmarkdowntextedit)
mo2_configure_plugin(
        mo2_notes
        WARNINGS OFF
        PRIVATE_DEPENDS
        boost uibase
        Qt::WebEngineWidgets
)
mo2_install_target(mo2_notes)