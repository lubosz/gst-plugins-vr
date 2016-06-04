#version 330

in vec2 out_uv;
uniform sampler2D texture;
out vec4 frag_color;

void main()
{
  frag_color = vec4 (out_uv, 0, 1);
}
