attribute vec4 position;
uniform float aspect_ratio;
varying vec2 fractal_position;

void main()
{
  gl_Position = position;
  fractal_position = vec2(position.y * 0.5 - 0.3, aspect_ratio * position.x * 0.5);
  fractal_position *= 2.5;
}
