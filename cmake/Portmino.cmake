include(CheckCXXCompilerFlag)

set(PORTMINO_COMPILE_OPTIONS "")

function(_checked_add_compile_option FLAG VAR)
    check_cxx_compiler_flag(${FLAG} ${VAR})
    if(${VAR})
        set(PORTMINO_COMPILE_OPTIONS ${PORTMINO_COMPILE_OPTIONS} ${FLAG} PARENT_SCOPE)
    endif()
endfunction()

if(MSVC)
    list(APPEND PORTMINO_COMPILE_OPTIONS "-D_CRT_NONSTDC_NO_DEPRECATE")
    list(APPEND PORTMINO_COMPILE_OPTIONS "-D_CRT_SECURE_NO_WARNINGS")

    # Extra warnings for cl.exe
    _checked_add_compile_option(/we4013 W_4013) # Implicit function declaration
    _checked_add_compile_option(/we4133 W_4133) # Incompatible types
    _checked_add_compile_option(/we4477 W_4477) # Format string mismatch

    # Extra warnings for clang-cl.exe
    _checked_add_compile_option(-Wno-pragma-pack W_NO_PACK)
else()
    # Extra warnings for GCC
    _checked_add_compile_option(-Wrestrict W_RESTRICT)

    # Permissive C++, we need this for now
    _checked_add_compile_option(-fpermissive F_PERMISSIVE)
endif()

# These warnings exist in both GCC and Clang
_checked_add_compile_option(-Werror=implicit-function-declaration E_IMPLICIT_FUNC)
_checked_add_compile_option(-Werror=incompatible-pointer-types E_INCOMPAT_PTR)
_checked_add_compile_option(-Werror=int-conversion E_INT_CONV)
_checked_add_compile_option(-Wformat=2 W_FORMAT_2)
_checked_add_compile_option(-Wnull-dereference W_NULL_DEREF)

function(add_portmino_settings)
    foreach(target ${ARGN})
        set_target_properties(${target} PROPERTIES CXX_STANDARD 98)
        target_compile_options(${target} PRIVATE ${PORTMINO_COMPILE_OPTIONS})
    endforeach()
endfunction()
