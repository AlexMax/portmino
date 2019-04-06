include(CheckCCompilerFlag)

set(PORTMINO_COMPILE_OPTIONS "")

function(_checked_add_compile_option FLAG VAR)
    check_c_compiler_flag(${FLAG} ${VAR})
    if(${VAR})
        set(PORTMINO_COMPILE_OPTIONS ${PORTMINO_COMPILE_OPTIONS} ${FLAG} PARENT_SCOPE)
    endif()
endfunction()

if(MSVC)
    list(APPEND PORTMINO_COMPILE_OPTIONS "-D_CRT_NONSTDC_NO_DEPRECATE")
    list(APPEND PORTMINO_COMPILE_OPTIONS "-D_CRT_SECURE_NO_WARNINGS")

    # Extra warnings for clang-cl.exe
    _checked_add_compile_option(-Wno-pragma-pack W_NO_PACK)
else()
    # Extra warnings for GCC
    _checked_add_compile_option(-Wrestrict W_RESTRICT)
endif()

# These warnings exist in both GCC and Clang
_checked_add_compile_option(-Werror=implicit-function-declaration E_IMPLICIT_FUNC)
_checked_add_compile_option(-Werror=incompatible-pointer-types E_INCOMPAT_PTR)
_checked_add_compile_option(-Werror=int-conversion E_INT_CONV)
_checked_add_compile_option(-Wformat=2 W_FORMAT_2)
_checked_add_compile_option(-Wnull-dereference W_NULL_DEREF)

function(add_portmino_settings)
    foreach(target ${ARGN})
        target_compile_options(${target} PRIVATE ${PORTMINO_COMPILE_OPTIONS})
    endforeach()
endfunction()
