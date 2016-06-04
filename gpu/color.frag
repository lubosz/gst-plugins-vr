#version 330

in vec3 out_color;
out vec4 frag_color;

void main()
{
  frag_color = vec4 (out_color, 1);
}
