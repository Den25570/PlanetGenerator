#version 330 core

precision mediump float;
uniform mat4 u_projection;

layout (location = 0) in vec3 a_xyz;
layout (location = 1) in vec2 a_tm;
out vec2 v_tm;

void main() {
  v_tm = a_tm;
  gl_Position = u_projection * vec4(a_xyz, 1);
  //gl_Position = u_projection * vec4(a_xyz.x / (1.0 - a_xyz.z),a_xyz.y / (1.0 - a_xyz.z),0, 1);
}