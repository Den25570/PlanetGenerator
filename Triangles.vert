#version 330

precision mediump float;
uniform mat4 u_projection;
layout (location = 0) attribute vec3 a_xyz;
layout (location = 1) attribute vec2 a_tm;

out vec2 v_tm;

void main() {
  v_tm = a_tm;
  gl_Position = u_projection * vec4(a_xyz, 1);
}