#version 330

precision mediump float;
uniform mat4 u_projection;
uniform mat4 model;
layout (location = 0) attribute vec3 a_xyz;
layout (location = 1) attribute vec2 a_tm;

out vec2 v_tm;
out vec3 normal;
out vec3 frag_model_pos;

void main() {
  v_tm = a_tm;
  normal = a_xyz;
  frag_model_pos = (model * vec4(a_xyz, 1)).xyz;
  gl_Position = u_projection * vec4(a_xyz, 1);
}