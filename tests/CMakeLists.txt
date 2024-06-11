include_directories(../)
add_executable(test_exe test.c)
target_link_libraries(test_exe sunflower)
add_test(test test_exe)