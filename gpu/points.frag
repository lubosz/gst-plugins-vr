#version 330

in vec2 out_uv;
uniform sampler2D texture;

void main()
{
  gl_FragColor = texture2D (texture, out_uv);
}
