# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.25

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build"

# Include any dependencies generated for this target.
include tests/CMakeFiles/malloc_3_test.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include tests/CMakeFiles/malloc_3_test.dir/compiler_depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/malloc_3_test.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/malloc_3_test.dir/flags.make

tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_basic.cpp.o: tests/CMakeFiles/malloc_3_test.dir/flags.make
tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_basic.cpp.o: /home/student/Desktop/OS/HW4/tests/os_hw4_tests\ (1)/tests/malloc_3_test_basic.cpp
tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_basic.cpp.o: tests/CMakeFiles/malloc_3_test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_basic.cpp.o"
	cd "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_basic.cpp.o -MF CMakeFiles/malloc_3_test.dir/malloc_3_test_basic.cpp.o.d -o CMakeFiles/malloc_3_test.dir/malloc_3_test_basic.cpp.o -c "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/tests/malloc_3_test_basic.cpp"

tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_basic.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/malloc_3_test.dir/malloc_3_test_basic.cpp.i"
	cd "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/tests/malloc_3_test_basic.cpp" > CMakeFiles/malloc_3_test.dir/malloc_3_test_basic.cpp.i

tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_basic.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/malloc_3_test.dir/malloc_3_test_basic.cpp.s"
	cd "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/tests/malloc_3_test_basic.cpp" -o CMakeFiles/malloc_3_test.dir/malloc_3_test_basic.cpp.s

tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_reuse.cpp.o: tests/CMakeFiles/malloc_3_test.dir/flags.make
tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_reuse.cpp.o: /home/student/Desktop/OS/HW4/tests/os_hw4_tests\ (1)/tests/malloc_3_test_reuse.cpp
tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_reuse.cpp.o: tests/CMakeFiles/malloc_3_test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_reuse.cpp.o"
	cd "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_reuse.cpp.o -MF CMakeFiles/malloc_3_test.dir/malloc_3_test_reuse.cpp.o.d -o CMakeFiles/malloc_3_test.dir/malloc_3_test_reuse.cpp.o -c "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/tests/malloc_3_test_reuse.cpp"

tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_reuse.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/malloc_3_test.dir/malloc_3_test_reuse.cpp.i"
	cd "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/tests/malloc_3_test_reuse.cpp" > CMakeFiles/malloc_3_test.dir/malloc_3_test_reuse.cpp.i

tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_reuse.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/malloc_3_test.dir/malloc_3_test_reuse.cpp.s"
	cd "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/tests/malloc_3_test_reuse.cpp" -o CMakeFiles/malloc_3_test.dir/malloc_3_test_reuse.cpp.s

tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_scalloc.cpp.o: tests/CMakeFiles/malloc_3_test.dir/flags.make
tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_scalloc.cpp.o: /home/student/Desktop/OS/HW4/tests/os_hw4_tests\ (1)/tests/malloc_3_test_scalloc.cpp
tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_scalloc.cpp.o: tests/CMakeFiles/malloc_3_test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_scalloc.cpp.o"
	cd "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_scalloc.cpp.o -MF CMakeFiles/malloc_3_test.dir/malloc_3_test_scalloc.cpp.o.d -o CMakeFiles/malloc_3_test.dir/malloc_3_test_scalloc.cpp.o -c "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/tests/malloc_3_test_scalloc.cpp"

tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_scalloc.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/malloc_3_test.dir/malloc_3_test_scalloc.cpp.i"
	cd "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/tests/malloc_3_test_scalloc.cpp" > CMakeFiles/malloc_3_test.dir/malloc_3_test_scalloc.cpp.i

tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_scalloc.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/malloc_3_test.dir/malloc_3_test_scalloc.cpp.s"
	cd "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/tests/malloc_3_test_scalloc.cpp" -o CMakeFiles/malloc_3_test.dir/malloc_3_test_scalloc.cpp.s

tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_split_and_merge.cpp.o: tests/CMakeFiles/malloc_3_test.dir/flags.make
tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_split_and_merge.cpp.o: /home/student/Desktop/OS/HW4/tests/os_hw4_tests\ (1)/tests/malloc_3_test_split_and_merge.cpp
tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_split_and_merge.cpp.o: tests/CMakeFiles/malloc_3_test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_split_and_merge.cpp.o"
	cd "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_split_and_merge.cpp.o -MF CMakeFiles/malloc_3_test.dir/malloc_3_test_split_and_merge.cpp.o.d -o CMakeFiles/malloc_3_test.dir/malloc_3_test_split_and_merge.cpp.o -c "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/tests/malloc_3_test_split_and_merge.cpp"

tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_split_and_merge.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/malloc_3_test.dir/malloc_3_test_split_and_merge.cpp.i"
	cd "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/tests/malloc_3_test_split_and_merge.cpp" > CMakeFiles/malloc_3_test.dir/malloc_3_test_split_and_merge.cpp.i

tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_split_and_merge.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/malloc_3_test.dir/malloc_3_test_split_and_merge.cpp.s"
	cd "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/tests/malloc_3_test_split_and_merge.cpp" -o CMakeFiles/malloc_3_test.dir/malloc_3_test_split_and_merge.cpp.s

tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_srealloc.cpp.o: tests/CMakeFiles/malloc_3_test.dir/flags.make
tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_srealloc.cpp.o: /home/student/Desktop/OS/HW4/tests/os_hw4_tests\ (1)/tests/malloc_3_test_srealloc.cpp
tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_srealloc.cpp.o: tests/CMakeFiles/malloc_3_test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_srealloc.cpp.o"
	cd "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_srealloc.cpp.o -MF CMakeFiles/malloc_3_test.dir/malloc_3_test_srealloc.cpp.o.d -o CMakeFiles/malloc_3_test.dir/malloc_3_test_srealloc.cpp.o -c "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/tests/malloc_3_test_srealloc.cpp"

tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_srealloc.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/malloc_3_test.dir/malloc_3_test_srealloc.cpp.i"
	cd "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/tests/malloc_3_test_srealloc.cpp" > CMakeFiles/malloc_3_test.dir/malloc_3_test_srealloc.cpp.i

tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_srealloc.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/malloc_3_test.dir/malloc_3_test_srealloc.cpp.s"
	cd "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/tests/malloc_3_test_srealloc.cpp" -o CMakeFiles/malloc_3_test.dir/malloc_3_test_srealloc.cpp.s

tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_srealloc_cases.cpp.o: tests/CMakeFiles/malloc_3_test.dir/flags.make
tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_srealloc_cases.cpp.o: /home/student/Desktop/OS/HW4/tests/os_hw4_tests\ (1)/tests/malloc_3_test_srealloc_cases.cpp
tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_srealloc_cases.cpp.o: tests/CMakeFiles/malloc_3_test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_srealloc_cases.cpp.o"
	cd "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_srealloc_cases.cpp.o -MF CMakeFiles/malloc_3_test.dir/malloc_3_test_srealloc_cases.cpp.o.d -o CMakeFiles/malloc_3_test.dir/malloc_3_test_srealloc_cases.cpp.o -c "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/tests/malloc_3_test_srealloc_cases.cpp"

tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_srealloc_cases.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/malloc_3_test.dir/malloc_3_test_srealloc_cases.cpp.i"
	cd "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/tests/malloc_3_test_srealloc_cases.cpp" > CMakeFiles/malloc_3_test.dir/malloc_3_test_srealloc_cases.cpp.i

tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_srealloc_cases.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/malloc_3_test.dir/malloc_3_test_srealloc_cases.cpp.s"
	cd "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/tests/malloc_3_test_srealloc_cases.cpp" -o CMakeFiles/malloc_3_test.dir/malloc_3_test_srealloc_cases.cpp.s

tests/CMakeFiles/malloc_3_test.dir/__/malloc_3.cpp.o: tests/CMakeFiles/malloc_3_test.dir/flags.make
tests/CMakeFiles/malloc_3_test.dir/__/malloc_3.cpp.o: /home/student/Desktop/OS/HW4/tests/os_hw4_tests\ (1)/malloc_3.cpp
tests/CMakeFiles/malloc_3_test.dir/__/malloc_3.cpp.o: tests/CMakeFiles/malloc_3_test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object tests/CMakeFiles/malloc_3_test.dir/__/malloc_3.cpp.o"
	cd "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT tests/CMakeFiles/malloc_3_test.dir/__/malloc_3.cpp.o -MF CMakeFiles/malloc_3_test.dir/__/malloc_3.cpp.o.d -o CMakeFiles/malloc_3_test.dir/__/malloc_3.cpp.o -c "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/malloc_3.cpp"

tests/CMakeFiles/malloc_3_test.dir/__/malloc_3.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/malloc_3_test.dir/__/malloc_3.cpp.i"
	cd "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/malloc_3.cpp" > CMakeFiles/malloc_3_test.dir/__/malloc_3.cpp.i

tests/CMakeFiles/malloc_3_test.dir/__/malloc_3.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/malloc_3_test.dir/__/malloc_3.cpp.s"
	cd "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/malloc_3.cpp" -o CMakeFiles/malloc_3_test.dir/__/malloc_3.cpp.s

# Object files for target malloc_3_test
malloc_3_test_OBJECTS = \
"CMakeFiles/malloc_3_test.dir/malloc_3_test_basic.cpp.o" \
"CMakeFiles/malloc_3_test.dir/malloc_3_test_reuse.cpp.o" \
"CMakeFiles/malloc_3_test.dir/malloc_3_test_scalloc.cpp.o" \
"CMakeFiles/malloc_3_test.dir/malloc_3_test_split_and_merge.cpp.o" \
"CMakeFiles/malloc_3_test.dir/malloc_3_test_srealloc.cpp.o" \
"CMakeFiles/malloc_3_test.dir/malloc_3_test_srealloc_cases.cpp.o" \
"CMakeFiles/malloc_3_test.dir/__/malloc_3.cpp.o"

# External object files for target malloc_3_test
malloc_3_test_EXTERNAL_OBJECTS =

tests/malloc_3_test: tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_basic.cpp.o
tests/malloc_3_test: tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_reuse.cpp.o
tests/malloc_3_test: tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_scalloc.cpp.o
tests/malloc_3_test: tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_split_and_merge.cpp.o
tests/malloc_3_test: tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_srealloc.cpp.o
tests/malloc_3_test: tests/CMakeFiles/malloc_3_test.dir/malloc_3_test_srealloc_cases.cpp.o
tests/malloc_3_test: tests/CMakeFiles/malloc_3_test.dir/__/malloc_3.cpp.o
tests/malloc_3_test: tests/CMakeFiles/malloc_3_test.dir/build.make
tests/malloc_3_test: _deps/catch2-build/src/libCatch2Main.a
tests/malloc_3_test: _deps/catch2-build/src/libCatch2.a
tests/malloc_3_test: tests/CMakeFiles/malloc_3_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir="/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_8) "Linking CXX executable malloc_3_test"
	cd "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests" && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/malloc_3_test.dir/link.txt --verbose=$(VERBOSE)
	cd "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests" && /usr/bin/cmake -D TEST_TARGET=malloc_3_test -D "TEST_EXECUTABLE=/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests/malloc_3_test" -D TEST_EXECUTOR= -D "TEST_WORKING_DIR=/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests" -D TEST_SPEC= -D TEST_EXTRA_ARGS= -D TEST_PROPERTIES= -D TEST_PREFIX=malloc_3. -D TEST_SUFFIX= -D TEST_LIST=malloc_3_test_TESTS -D TEST_REPORTER= -D TEST_OUTPUT_DIR= -D TEST_OUTPUT_PREFIX= -D TEST_OUTPUT_SUFFIX= -D "CTEST_FILE=/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests/malloc_3_test_tests-b12d07c.cmake" -P "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/_deps/catch2-src/extras/CatchAddTests.cmake"

# Rule to build all files generated by this target.
tests/CMakeFiles/malloc_3_test.dir/build: tests/malloc_3_test
.PHONY : tests/CMakeFiles/malloc_3_test.dir/build

tests/CMakeFiles/malloc_3_test.dir/clean:
	cd "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests" && $(CMAKE_COMMAND) -P CMakeFiles/malloc_3_test.dir/cmake_clean.cmake
.PHONY : tests/CMakeFiles/malloc_3_test.dir/clean

tests/CMakeFiles/malloc_3_test.dir/depend:
	cd "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build" && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)" "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/tests" "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build" "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests" "/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests/CMakeFiles/malloc_3_test.dir/DependInfo.cmake" --color=$(COLOR)
.PHONY : tests/CMakeFiles/malloc_3_test.dir/depend
