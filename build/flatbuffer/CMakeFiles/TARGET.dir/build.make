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

# Utility rule file for TARGET.

# Include the progress variables for this target.
include flatbuffer/CMakeFiles/TARGET.dir/progress.make

flatbuffer/CMakeFiles/TARGET: ../flatbuffer/net.fbs
	cd /home/gyz/grit/build/flatbuffer && /home/gyz/grit/build/transaction/gtm/gtm PRE_BUILD
	cd /home/gyz/grit/build/flatbuffer && /home/gyz/grit/third_party/bin/flatc -c -b /home/gyz/grit/flatbuffer/net.fbs

TARGET: flatbuffer/CMakeFiles/TARGET
TARGET: flatbuffer/CMakeFiles/TARGET.dir/build.make

.PHONY : TARGET

# Rule to build all files generated by this target.
flatbuffer/CMakeFiles/TARGET.dir/build: TARGET

.PHONY : flatbuffer/CMakeFiles/TARGET.dir/build

flatbuffer/CMakeFiles/TARGET.dir/clean:
	cd /home/gyz/grit/build/flatbuffer && $(CMAKE_COMMAND) -P CMakeFiles/TARGET.dir/cmake_clean.cmake
.PHONY : flatbuffer/CMakeFiles/TARGET.dir/clean

flatbuffer/CMakeFiles/TARGET.dir/depend:
	cd /home/gyz/grit/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/gyz/grit /home/gyz/grit/flatbuffer /home/gyz/grit/build /home/gyz/grit/build/flatbuffer /home/gyz/grit/build/flatbuffer/CMakeFiles/TARGET.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : flatbuffer/CMakeFiles/TARGET.dir/depend

