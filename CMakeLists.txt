cmake_minimum_required (VERSION 2.8.11)
project (car-track)

set(PROJECT_EXE car-track)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -Wall")

find_package(GLEW REQUIRED)
find_package(GLM REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenCV2 REQUIRED)
find_package(GLOG REQUIRED)

include_directories(
    ${GLM_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIR}
    ${OpenGL_INCLUDE_DIR}
    ${OpenCV2_INCLUDE_DIR}
    ${GLOG_INCLUDE_DIRS}
)

include_directories(include)

file(GLOB_RECURSE SRC_FILES src/*.cpp include/*.hpp include/*.h)
add_executable(${PROJECT_EXE} ${SRC_FILES})

target_link_libraries(
    ${PROJECT_EXE}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARY}
    ${GLOG_LIBRARIES}
    ${OpenCV2_LIBRARIES}
)