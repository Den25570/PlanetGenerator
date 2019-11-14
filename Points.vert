#version 330

precision mediump float;

layout (location = 0) attribute vec3 a_xyz;

uniform mat4 u_projection;
uniform float u_pointsize;

void main() {
  gl_Position = u_projection * vec4(a_xyz, 1);
  gl_PointSize = gl_Position.z > 0.0? 0.0 : u_pointsize;
}