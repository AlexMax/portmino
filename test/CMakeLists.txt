set(TESTS
    test_platform_unix
    test_rulesetscript)

foreach(TEST ${TESTS})
    add_executable(${TEST} "${TEST}.c")
    target_link_libraries(${TEST} portmino-core basemino cmocka-x)
    add_test(${TEST} ${TEST})
endforeach()
