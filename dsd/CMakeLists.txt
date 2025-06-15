if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CARGO_CMD cargo build --verbose)
    set(TARGET_DIR "debug")
else ()
    set(CARGO_CMD cargo build --release --verbose)
    set(TARGET_DIR "release")
endif ()

# If the user is cross compiling to a different target, update the location of the target directory
# *Note*: Make sure to install the target with rustup, as well as setting the proper environment variables, such as CC, HOST_CC, CARGO_TARGET_<triple>_LINKER, ...
# See https://doc.rust-lang.org/cargo/reference/environment-variables.html#configuration-environment-variables
if (DEFINED ENV{CARGO_BUILD_TARGET})
    set(TARGET_DIR "$ENV{CARGO_BUILD_TARGET}/${TARGET_DIR}")
endif()

if(ENABLE_LTO)
    set(RUST_FLAGS "-Clinker-plugin-lto" "-Clinker=clang-17" "-Clink-arg=-fuse-ld=lld-17")
endif()

set(DSD_MELONDS_LIB "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_DIR}/libdsd_melonds_rs.a")

set(DSD_MELONDS_CXX "${CMAKE_CURRENT_BINARY_DIR}/dsd_melonds.cpp")
add_library(dsd_melonds STATIC ${DSD_MELONDS_CXX})
add_custom_command(
    OUTPUT ${DSD_MELONDS_CXX} ${DSD_MELONDS_LIB} dsd_melonds_always_run # Let cargo handle incremental builds
    COMMAND ${CMAKE_COMMAND} -E env CARGO_TARGET_DIR="${CMAKE_CURRENT_BINARY_DIR}" RUSTFLAGS="${RUST_FLAGS}" ${CARGO_CMD}
    COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/cxxbridge/dsd-melonds/src/lib.rs.cc ${DSD_MELONDS_CXX}
    COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/cxxbridge/dsd-melonds/src/lib.rs.h ${CMAKE_CURRENT_BINARY_DIR}/dsd_melonds.h
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(dsd_melonds pthread ${DSD_MELONDS_LIB})

if(UNIX)
    target_link_libraries(dsd_melonds dl)
elseif(WIN32)
    target_link_libraries(dsd_melonds ntdll)
endif()
