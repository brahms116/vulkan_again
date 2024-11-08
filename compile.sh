#!/bin/bash

set -e

/usr/local/bin/glslc ./shaders/simple_shader.vert -o ./shaders/simple_shader.vert.spv
/usr/local/bin/glslc ./shaders/simple_shader.frag -o ./shaders/simple_shader.frag.spv

/usr/local/bin/glslc ./shaders/point_light_shader.vert -o ./shaders/point_light_shader.vert.spv
/usr/local/bin/glslc ./shaders/point_light_shader.frag -o ./shaders/point_light_shader.frag.spv

/usr/local/bin/glslc ./shaders/shadow.vert -o ./shaders/shadow.vert.spv

