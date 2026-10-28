find_package(Qt5 REQUIRED COMPONENTS Test)

foreach(test_name tst_annotations tst_rasterize tst_serialization)
    add_executable(${test_name} ${test_name}.cpp)
    target_link_libraries(${test_name} PRIVATE annotator_core Qt5::Test)
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
