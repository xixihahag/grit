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
include transaction/gtm/CMakeFiles/gtm.dir/depend.make

# Include the progress variables for this target.
include transaction/gtm/CMakeFiles/gtm.dir/progress.make

# Include the compile flags for this target's objects.
include transaction/gtm/CMakeFiles/gtm.dir/flags.make

transaction/gtm/CMakeFiles/gtm.dir/gtmmain.cc.o: transaction/gtm/CMakeFiles/gtm.dir/flags.make
transaction/gtm/CMakeFiles/gtm.dir/gtmmain.cc.o: ../transaction/gtm/gtmmain.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gyz/grit/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object transaction/gtm/CMakeFiles/gtm.dir/gtmmain.cc.o"
	cd /home/gyz/grit/build/transaction/gtm && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/gtm.dir/gtmmain.cc.o -c /home/gyz/grit/transaction/gtm/gtmmain.cc

transaction/gtm/CMakeFiles/gtm.dir/gtmmain.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/gtm.dir/gtmmain.cc.i"
	cd /home/gyz/grit/build/transaction/gtm && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gyz/grit/transaction/gtm/gtmmain.cc > CMakeFiles/gtm.dir/gtmmain.cc.i

transaction/gtm/CMakeFiles/gtm.dir/gtmmain.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/gtm.dir/gtmmain.cc.s"
	cd /home/gyz/grit/build/transaction/gtm && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gyz/grit/transaction/gtm/gtmmain.cc -o CMakeFiles/gtm.dir/gtmmain.cc.s

transaction/gtm/CMakeFiles/gtm.dir/gtm.cc.o: transaction/gtm/CMakeFiles/gtm.dir/flags.make
transaction/gtm/CMakeFiles/gtm.dir/gtm.cc.o: ../transaction/gtm/gtm.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gyz/grit/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object transaction/gtm/CMakeFiles/gtm.dir/gtm.cc.o"
	cd /home/gyz/grit/build/transaction/gtm && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/gtm.dir/gtm.cc.o -c /home/gyz/grit/transaction/gtm/gtm.cc

transaction/gtm/CMakeFiles/gtm.dir/gtm.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/gtm.dir/gtm.cc.i"
	cd /home/gyz/grit/build/transaction/gtm && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gyz/grit/transaction/gtm/gtm.cc > CMakeFiles/gtm.dir/gtm.cc.i

transaction/gtm/CMakeFiles/gtm.dir/gtm.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/gtm.dir/gtm.cc.s"
	cd /home/gyz/grit/build/transaction/gtm && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gyz/grit/transaction/gtm/gtm.cc -o CMakeFiles/gtm.dir/gtm.cc.s

# Object files for target gtm
gtm_OBJECTS = \
"CMakeFiles/gtm.dir/gtmmain.cc.o" \
"CMakeFiles/gtm.dir/gtm.cc.o"

# External object files for target gtm
gtm_EXTERNAL_OBJECTS =

transaction/gtm/gtm: transaction/gtm/CMakeFiles/gtm.dir/gtmmain.cc.o
transaction/gtm/gtm: transaction/gtm/CMakeFiles/gtm.dir/gtm.cc.o
transaction/gtm/gtm: transaction/gtm/CMakeFiles/gtm.dir/build.make
transaction/gtm/gtm: libbase.so
transaction/gtm/gtm: transaction/gtm/CMakeFiles/gtm.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/gyz/grit/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable gtm"
	cd /home/gyz/grit/build/transaction/gtm && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/gtm.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
transaction/gtm/CMakeFiles/gtm.dir/build: transaction/gtm/gtm

.PHONY : transaction/gtm/CMakeFiles/gtm.dir/build

transaction/gtm/CMakeFiles/gtm.dir/clean:
	cd /home/gyz/grit/build/transaction/gtm && $(CMAKE_COMMAND) -P CMakeFiles/gtm.dir/cmake_clean.cmake
.PHONY : transaction/gtm/CMakeFiles/gtm.dir/clean

transaction/gtm/CMakeFiles/gtm.dir/depend:
	cd /home/gyz/grit/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/gyz/grit /home/gyz/grit/transaction/gtm /home/gyz/grit/build /home/gyz/grit/build/transaction/gtm /home/gyz/grit/build/transaction/gtm/CMakeFiles/gtm.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : transaction/gtm/CMakeFiles/gtm.dir/depend

