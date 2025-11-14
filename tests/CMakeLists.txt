# CMakeList.txt : CMake project for core_as, include source and define
# project specific logic here.
#
add_executable(test_rex test_rex.cpp)
target_link_libraries(test_rex simrex::simrex GTest::gtest_main)

add_test(NAME test_rex COMMAND test_rex)

if (EMSCRIPTEN)
    set_target_properties (test_rex PROPERTIES SUFFIX .html)
endif(EMSCRIPTEN)
