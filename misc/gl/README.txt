#!/bin/bsh

#
# Dependences: 
# glfw: A multi-platform library for OpenGL, window and input
# GLEW: The OpenGL Extension Wrangler Library
# SOIL: Simple OpenGL Image Library.
# GLM: OpenGL Mathematics is a header only C++ mathematics library for graphics software based on the OpenGL Shading Language (GLSL) specifications
#
# Building examples: 
# 1) Enter each holder 
# 2) Run command: /bin/bash ../README.txt
#

LDFLAGS="`pkg-config --static --libs x11 xrandr xi xxf86vm glew glfw3` -lSOIL"
CXXFLAGS="-I/usr/include/SOIL -I../common"
CPPFLAGS="-g -std=c++11"
CXX=g++

$CXX $CPPFLAGS $CXXFLAGS *.cpp $LDFLAGS

