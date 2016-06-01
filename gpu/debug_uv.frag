#version 330

in vec2 out_uv;
in vec3 out_pos;
uniform sampler2D texture;

void main()
{
  gl_FragColor = vec4 (out_uv, 0, 1);
}
