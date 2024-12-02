add_custom_target(tests_output_artifacts)

# Add test for OutputArtifacts library
# .cpp file must have the name of target
function(add_output_artifacts_test target)
    add_executable(${target} ${target}.cpp)

    target_link_libraries(${target} PRIVATE proof_generatorOutputArtifacts)
    target_link_libraries(${target} PRIVATE GTest::gtest GTest::gtest_main)

    set_target_properties(${target} PROPERTIES
        LINKER_LANGUAGE CXX
        EXPORT_NAME ${target}
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED TRUE
    )

    gtest_discover_tests(${target})
    add_dependencies(tests_output_artifacts ${target})
endfunction()

add_output_artifacts_test(test_range)
add_output_artifacts_test(test_ranges)
add_output_artifacts_test(test_circuit_writer)
add_output_artifacts_test(test_assignment_table_writer)

file(INSTALL "resources" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
