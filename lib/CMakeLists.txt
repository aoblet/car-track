target_include_directories(${PROJECT_EXE} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB IMGUI_SRC imgui/*.cpp)

add_library(IMGUI_LIBRARY STATIC ${IMGUI_SRC})
