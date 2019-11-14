#version 330 core

//layout (location = 0) in vec3 position;
//layout (location = 1) in vec3 input_color;

//uniform mat4 MVP;

//out vec3 color;

//void main()
//{
//	gl_Position = MVP * vec4(position, 1.0f);
//	color = input_color;
//}

precision mediump float;
uniform mat4 u_projection;

layout (location = 0) attribute vec3 a_xyz;
layout (location = 1) attribute vec2 a_tm;
varying vec2 v_tm;

void main() {
  v_tm = a_tm;
  gl_Position = u_projection * vec4(a_xyz, 1);
}