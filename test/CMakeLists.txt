set(TESTS
    test_platform_unix
    test_rulesetscript)

foreach(TEST ${TESTS})
    add_executable(${TEST} "${TEST}.c")
    add_sanitizers(${TEST})
    target_link_libraries(${TEST} portmino-core basemino cmocka-x)
    add_test(${TEST} ${TEST})
endforeach()
