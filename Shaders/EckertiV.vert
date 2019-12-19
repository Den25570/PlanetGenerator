#version 330 core

precision highp float;
uniform mat4 u_projection;
uniform mat4 model;
layout (location = 0) attribute vec3 a_xyz;
layout (location = 1) attribute vec2 a_tm;

out vec2 v_tm;
out vec3 normal;
out vec3 frag_model_pos;


void main() {
  float r = 1.0f;
  float latitude = 1/(tan(a_xyz.y/a_xyz.x));
  float longtitude = 1/(cos(a_xyz.z/r));
  
  float x = 0;
  float y = 1.25 * log(tan(3.14+0.4*latitude));

  v_tm = a_tm;
  normal = (model * vec4(a_xyz, 1)).xyz;
  frag_model_pos = (model * vec4(a_xyz, 1)).xyz;
  gl_Position = u_projection * vec4(x,y,0,1);
}