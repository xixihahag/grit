# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.15

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /opt/cmake-3.15.0/bin/cmake

# The command to remove a file.
RM = /opt/cmake-3.15.0/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/gyz/grit

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/gyz/grit/build

# Include any dependencies generated for this target.
include transaction/gtm/CMakeFiles/test.dir/depend.make

# Include the progress variables for this target.
include transaction/gtm/CMakeFiles/test.dir/progress.make

# Include the compile flags for this target's objects.
include transaction/gtm/CMakeFiles/test.dir/flags.make

transaction/gtm/CMakeFiles/test.dir/test.cc.o: transaction/gtm/CMakeFiles/test.dir/flags.make
transaction/gtm/CMakeFiles/test.dir/test.cc.o: ../transaction/gtm/test.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gyz/grit/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object transaction/gtm/CMakeFiles/test.dir/test.cc.o"
	cd /home/gyz/grit/build/transaction/gtm && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/test.dir/test.cc.o -c /home/gyz/grit/transaction/gtm/test.cc

transaction/gtm/CMakeFiles/test.dir/test.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test.dir/test.cc.i"
	cd /home/gyz/grit/build/transaction/gtm && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gyz/grit/transaction/gtm/test.cc > CMakeFiles/test.dir/test.cc.i

transaction/gtm/CMakeFiles/test.dir/test.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test.dir/test.cc.s"
	cd /home/gyz/grit/build/transaction/gtm && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gyz/grit/transaction/gtm/test.cc -o CMakeFiles/test.dir/test.cc.s

# Object files for target test
test_OBJECTS = \
"CMakeFiles/test.dir/test.cc.o"

# External object files for target test
test_EXTERNAL_OBJECTS =

transaction/gtm/test: transaction/gtm/CMakeFiles/test.dir/test.cc.o
transaction/gtm/test: transaction/gtm/CMakeFiles/test.dir/build.make
transaction/gtm/test: transaction/gtm/CMakeFiles/test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/gyz/grit/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable test"
	cd /home/gyz/grit/build/transaction/gtm && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
transaction/gtm/CMakeFiles/test.dir/build: transaction/gtm/test

.PHONY : transaction/gtm/CMakeFiles/test.dir/build

transaction/gtm/CMakeFiles/test.dir/clean:
	cd /home/gyz/grit/build/transaction/gtm && $(CMAKE_COMMAND) -P CMakeFiles/test.dir/cmake_clean.cmake
.PHONY : transaction/gtm/CMakeFiles/test.dir/clean

transaction/gtm/CMakeFiles/test.dir/depend:
	cd /home/gyz/grit/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/gyz/grit /home/gyz/grit/transaction/gtm /home/gyz/grit/build /home/gyz/grit/build/transaction/gtm /home/gyz/grit/build/transaction/gtm/CMakeFiles/test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : transaction/gtm/CMakeFiles/test.dir/depend
