
add_executable(volume volume.cpp)
target_link_libraries(volume ${OSPRAY_LIBRARIES})
add_dependencies(volume stb)

add_executable(box box.cpp)
target_link_libraries(box ${TBB_IMPORTED_TARGETS})
target_link_libraries(box ospray::ospray rkcommon::rkcommon TBB::tbb)
add_dependencies(box stb)
