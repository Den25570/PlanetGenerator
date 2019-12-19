#version 330

precision mediump float;
uniform vec4 u_multiply_rgba, u_add_rgba;
in vec4 v_rgba;
out vec4 Color;

uniform float ambitient_strength;
uniform vec3 light_color;
uniform vec3 light_pos;

in vec3 normal;
in vec3 frag_model_pos;

vec3 processLight() {
   vec3 ambitient_light = ambitient_strength * light_color;
   vec3 norm = normalize(normal);
   vec3 lightDir = normalize(light_pos - frag_model_pos);
   float diff = max(dot(norm, lightDir), 0.0);
   vec3 diffuse_light = diff * light_color;
   vec3 light = ambitient_light + diffuse_light;
   return light;
}

void main() {
   Color = (v_rgba * u_multiply_rgba + u_add_rgba) * vec4(processLight(),1.0f);
}

