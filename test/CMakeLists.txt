# Test targets

if(MSVC)
    set(UH_WARNING_OPTIONS /W4)
else()
    set(UH_WARNING_OPTIONS -Wall -Wextra)
endif()

add_executable(uhash-test "test.c")
target_compile_options(uhash-test PRIVATE ${UH_WARNING_OPTIONS})
target_link_libraries(uhash-test PRIVATE uhash)

add_executable(uhash-bench "bench.c")
target_compile_options(uhash-bench PRIVATE ${UH_WARNING_OPTIONS})
target_link_libraries(uhash-bench PRIVATE uhash)
