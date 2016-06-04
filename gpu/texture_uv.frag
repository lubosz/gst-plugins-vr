#version 330

in vec2 out_uv;
uniform sampler2D texture;
out vec4 frag_color;

void main()
{
  frag_color = texture2D (texture, out_uv);
}
