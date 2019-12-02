#version 330

precision mediump float;
uniform mat4 u_projection;
layout (location = 0) attribute  vec3 a_xyz;
layout (location = 1) attribute  vec4 a_rgba;
varying vec4 v_rgba;

void main() {
  vec4 pos = u_projection * vec4(a_xyz, 1);
  v_rgba = (-2.0 * pos.z) * a_rgba;
  gl_Position = pos;
}