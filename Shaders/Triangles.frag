#version 330

precision mediump float;

uniform sampler2D u_colormap;
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

   vec3 color = texture2D(u_colormap, vec2(e, v_tm.y)).rgb;
   if (v_tm.z >= 0.729 )
		if (v_tm.x > 0) color = vec3(1.1);
		else color += 1.0f;
   if (v_tm.x > 0 && v_tm.z >= 0.512)
		color = vec3(1.4);
   
   color *= processLight();
   BloomColor = processBloomColor(color) * vec4(processLight(), 1.0);
   FragColor = vec4(color, 1.0f);
}