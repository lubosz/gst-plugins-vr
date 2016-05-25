#version 330

in vec2 fractal_position;
in vec2 out_uv;

uniform float time;

const vec4 K = vec4(1.0, 0.66, 0.33, 3.0);


vec4 hsv_to_rgb(float hue, float saturation, float value) {
  vec4 p = abs(fract(vec4(hue) + K) * 6.0 - K.wwww);
  return value * mix(K.xxxx, clamp(p - K.xxxx, 0.0, 1.0), saturation);
}

vec4 i_to_rgb(int i) {
  float hue = float(i) / 100.0 + sin(time);
  return hsv_to_rgb(hue, 0.5, 0.8);
}

vec2 pow_2_complex(vec2 c) {
  return vec2(c.x*c.x - c.y*c.y, 2.0 * c.x * c.y);
}

vec2 mandelbrot(vec2 c, vec2 c0) {
  return pow_2_complex(c) + c0;
}

vec4 iterate_pixel(vec2 position) {
  vec2 c = vec2(0);
  for (int i=0; i < 20; i++) {
    if (c.x*c.x + c.y*c.y > 2.0*2.0)
      return i_to_rgb(i);
    c = mandelbrot(c, position);
  }
  return vec4(0, 0, 0, 1);
}

void main() {
  gl_FragColor = iterate_pixel(fractal_position);
  //gl_FragColor = vec4(fractal_position,0,1);
}
