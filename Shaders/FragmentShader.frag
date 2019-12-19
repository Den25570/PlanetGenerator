#version 330 core

#extension GL_OES_standard_derivatives : enable

precision mediump float;

uniform sampler2D u_colormap;
uniform vec2 u_light_angle;
uniform float u_inverse_texture_size, u_slope, u_flat, u_c, u_d, u_outline_strength;

uniform float ambitient_strength;
uniform vec3 light_color;
uniform vec3 light_pos;

in vec3 v_tm;
in vec3 normal;
in vec3 frag_model_pos;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BloomColor;

vec3 processLight() {
   vec3 ambitient_light = ambitient_strength * light_color;
   vec3 norm = normalize(normal);
   vec3 lightDir = normalize(light_pos - frag_model_pos);
   float diff = max(dot(norm, lightDir), 0.0);
   vec3 diffuse_light = diff * light_color;
   vec3 light = ambitient_light + diffuse_light;
   return light;
}

vec4 processBloomColor(vec3 color) {
	vec4 ResultColor;

	float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
	if(brightness > 0.0f)
       ResultColor = vec4(9/2550.0f, 105/2550.0f, 162/2550.0f, 0.1f);
   else
       ResultColor = vec4(0.0, 0.0, 0.0, 0.0);

	return ResultColor;
}

void main() {
   float e = v_tm.x > 0.0? 0.5 * (v_tm.x * v_tm.x + 1.0) : 0.5 * (v_tm.x + 1.0);
   e = e > 0.98 ? 0.98 : e < 0.02 ? 0.02 : e;
   float dedx = dFdx(v_tm.x);
   float dedy = dFdy(v_tm.x);
   vec3 slope_vector = normalize(vec3(dedy, dedx, u_d * 2.0 * u_inverse_texture_size));
   vec3 light_vector = normalize(vec3(u_light_angle, mix(u_slope, u_flat, slope_vector.z)));
   float outline = 1.0 + u_outline_strength * max(dedx,dedy);

   vec3 color = texture2D(u_colormap, vec2(e, v_tm.y)).rgb * processLight() / outline;
   BloomColor = processBloomColor(color) * vec4(processLight(),1.0);
   FragColor = vec4(color, 1.0f);
}