#version 330

in vec3 position;
in vec2 uv;

uniform float aspect_ratio;
uniform mat4 mvp;

out vec2 fractal_position;
out vec2 out_uv;

void main()
{
  gl_Position = mvp * vec4(position, 1);
  fractal_position = vec2(position.x * 0.5 - 0.3, position.y * 0.5);
  fractal_position *= 3.5;
  /*
  out_uv = uv;
  */
  out_uv = position.xy;
}
