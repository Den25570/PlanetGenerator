#version 330

precision mediump float;

uniform sampler2D u_colormap;
in vec2 v_tm;
void main() {
   float e = v_tm.x > 0.0? 0.5 * (v_tm.x * v_tm.x + 1.0) : 0.5 * (v_tm.x + 1.0);
    e = e > 0.98 ? 0.98 : e < 0.02 ? 0.02 : e;
   gl_FragColor = vec4(texture2D(u_colormap, vec2(e, v_tm.y)).rgb, 1);
}