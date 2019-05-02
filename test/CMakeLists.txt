set(TESTS
    test_environment
    test_globalscript
    test_proto
    test_protoscript
    test_ruleset
    test_script
    test_softblock
    test_vfs)
if(UNIX)
    list(APPEND TESTS test_platform_unix)
endif()

foreach(TEST ${TESTS})
    add_executable(${TEST} "${TEST}.c")
    add_sanitizers(${TEST})
    target_link_libraries(${TEST} portmino-core basemino cmocka-x)
    add_test(${TEST} ${TEST})
endforeach()
