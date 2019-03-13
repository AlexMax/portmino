set(TESTS
    test_rulesetscript
    test_scriptenv)
if(UNIX)
    list(APPEND TESTS test_platform_unix)
endif()

foreach(TEST ${TESTS})
    add_executable(${TEST} "${TEST}.c")
    add_sanitizers(${TEST})
    target_link_libraries(${TEST} portmino-core basemino cmocka-x)
    add_test(${TEST} ${TEST})
endforeach()
