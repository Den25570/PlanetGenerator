#version 330

precision mediump float;
uniform mat4 u_projection;
uniform mat4 model;

layout (location = 0) in  vec3 a_xyz;
layout (location = 1) in  vec4 a_rgba;

out vec3 normal;
out vec3 frag_model_pos;
out vec4 v_rgba;

void main() {
  vec4 pos = u_projection * vec4(a_xyz, 1);
  v_rgba = /*(-2.0 * pos.z) **/ a_rgba;
  normal = (model * vec4(a_xyz, 1)).xyz;
  frag_model_pos = (model * vec4(a_xyz, 1)).xyz;
  gl_Position = pos;
  
}