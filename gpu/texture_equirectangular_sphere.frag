#version 330

in vec2 out_uv;
uniform sampler2D sampler_equirectangular;
out vec4 frag_color;

const float PI = 3.1416;

uniform mat4 view;
uniform mat4 vp;

void main()
{
	vec2 fragCoord = vec2(out_uv) * 2 - 1;
	vec4 viewDir = normalize(inverse(vp) * vec4(fragCoord, 1, 1));

  float u = atan(viewDir.x, -viewDir.z) / (2 * PI) + 0.5;
  float v = acos(-viewDir.y) / PI;

	frag_color = texture(sampler_equirectangular, vec2(u, v));
}
