
find_package(ospray REQUIRED)
include_directories(${OSPRAY_INCLUDE_DIRS} ${STB_INCLUDES})

add_executable(volume volume.cpp)
target_link_libraries(volume ${OSPRAY_LIBRARIES})
add_dependencies(volume stb)
