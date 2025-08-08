add_library(metal_cpp ${CMAKE_CURRENT_SOURCE_DIR}/metal_cpp.cpp)

target_include_directories(metal_cpp PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/metal-cpp"
        )

target_compile_features(metal_cpp  PRIVATE cxx_std_17)


target_link_libraries(metal_cpp
    ${METAL_FRAMEWORK}
    ${FOUNDATION_FRAMEWORK}
    ${QUARTZCORE_FRAMEWORK}
)