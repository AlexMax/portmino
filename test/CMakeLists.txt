set(TESTS
    test_entity
    test_environment
    test_globalscript
    test_proto
    test_protoscript
    test_ruleset
    test_script
    test_serialize
    test_softblock
    test_vfs)
if(UNIX)
    list(APPEND TESTS test_platform_unix)
endif()

foreach(TEST ${TESTS})
    add_executable(${TEST} "${TEST}.c")
    add_portmino_settings(${TEST})
    add_sanitizers(${TEST})
    target_link_libraries(${TEST} portmino-core basemino cmocka-x)
    add_test(${TEST} ${TEST})
    set_tests_properties(${TEST} PROPERTIES
        ENVIRONMENT "LSAN_OPTIONS=suppressions=${CMAKE_SOURCE_DIR}/lsansupp.txt")
endforeach()
