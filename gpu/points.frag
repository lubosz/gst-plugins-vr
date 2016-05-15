#version 330

in vec2 out_uv;
uniform sampler2D texture;

void main()
{
  //gl_FragColor = vec4(1,0,0,1);
  gl_FragColor = texture2D (texture, out_uv);
}
