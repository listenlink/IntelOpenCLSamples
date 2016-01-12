include_directories(${COMMON_DIR})

add_executable(MotionEstimationAdvanced main.cpp ${BASIC_FILES} ${COMMON_DIR}/utils.cpp ${COMMON_DIR}/yuv_utils.cpp)
target_link_libraries(MotionEstimationAdvanced ${OPENCL_LIBRARIES})

configure_file(video_1920x1080_5frames.yuv ${CMAKE_CURRENT_BINARY_DIR}/video_1920x1080_5frames.yuv COPYONLY)
