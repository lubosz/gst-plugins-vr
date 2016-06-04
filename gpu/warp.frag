/* based on OpenHMD warp shader */

#version 330
precision highp float;

uniform sampler2D texture;
uniform vec2 screen_size;

const vec4 kappa = vec4(1.0, 0.22, 0.24, 0.0);
// const vec4 kappa = vec4(1.0, 1.7, 0.7, 15.0);

const vec2 screen_center_left = vec2(0.25, 0.5);
const vec2 screen_center_right = vec2(0.75, 0.5);

const vec2 scale = vec2(0.1469278, 0.2350845);
const vec2 scale_in = vec2(4, 2.5);

in vec2 out_uv;
out vec4 frag_color;

// scales input texture coordinates for distortion.
vec2 hmd_warp(vec2 lens_center)
{
	vec2 theta = (out_uv - lens_center) * scale_in; // scales to [-1, 1]
	float rSq = theta.x * theta.x + theta.y * theta.y;
	vec2 rvector = theta * (
	    kappa.x
	  + kappa.y * rSq
	  + kappa.z * rSq * rSq
	  + kappa.w * rSq * rSq * rSq);
	return lens_center + scale * rvector;
}

bool is_outside_area(vec2 tc, vec2 screen_center)
{
  const vec2 quarter_screen = vec2(0.25, 0.5);
  vec2 test = clamp(tc,
      screen_center - quarter_screen,
      screen_center + quarter_screen) - tc;
  return any(bvec2(test));
}

void main()
{
	// The following two variables need to be set per eye
	vec2 screen_center = out_uv.x < 0.5 ? screen_center_left : screen_center_right;
	vec2 tc = hmd_warp(screen_center);

	if (is_outside_area(tc, screen_center))
	{
	  // We are outside of the warped area
		frag_color = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}

  // double mono video for fake stereo
	//tc.x = gl_FragCoord.x < center_x ? (2.0 * tc.x) : (2.0 * (tc.x - 0.5));
	frag_color = texture2D(texture, tc);
}
