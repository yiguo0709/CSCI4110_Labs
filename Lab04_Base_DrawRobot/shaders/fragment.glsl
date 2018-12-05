#version 110
//uniform sampler2D u-depthBuffer;

varying vec4 v_colour;

void main() {
  //float depth = texture2D(u_depthBuffer, f_uv).x;
  gl_FragColor = v_colour;
}
