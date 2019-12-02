#version 330

precision mediump float;

uniform sampler2D u_colormap;
uniform float ambitient_strength;
uniform vec3 light_color;
uniform vec3 light_pos;

in vec2 v_tm;
in vec3 normal;
in vec3 frag_model_pos;

void main() {
   float e = v_tm.x > 0.0? 0.5 * (v_tm.x * v_tm.x + 1.0) : 0.5 * (v_tm.x + 1.0);
    e = e > 0.98 ? 0.98 : e < 0.02 ? 0.02 : e;

   vec3 ambitient_light = ambitient_strength * light_color;

   vec3 norm = normalize(normal);
   vec3 lightDir = normalize(light_pos - frag_model_pos);
   float diff = max(dot(norm, lightDir), 0.0);
   vec3 diffuse_light = diff * light_color;

   vec3 light = ambitient_light + diffuse_light;

   vec3 color = texture2D(u_colormap, vec2(e, v_tm.y)).rgb * light;
   gl_FragColor = vec4(color, 1.0f);
}