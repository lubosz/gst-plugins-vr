#version 330

// Taken from mts3d forums, from user fredrik.

uniform sampler2D texture;

uniform vec2 screen_size;

const vec2 lens_center_left = vec2(0.2863248, 0.5);
const vec2 lens_center_right = vec2(0.7136753, 0.5);
const vec2 screen_center_left = vec2(0.25, 0.5);
const vec2 screen_center_right = vec2(0.75, 0.5);
const vec2 scale = vec2(0.1469278, 0.2350845);
const vec2 scale_in = vec2(4, 2.5);
const vec4 kappa = vec4(1, 0.22, 0.24, 0);

in vec2 out_uv;
out vec4 fragColor;

// scales input texture coordinates for distortion.
vec2 HmdWarp(vec2 in01, vec2 LensCenter)
{
	vec2 theta = (in01 - LensCenter) * scale_in; // scales to [-1, 1]
	float rSq = theta.x * theta.x + theta.y * theta.y;
	vec2 rvector = theta * (kappa.x + kappa.y * rSq +
		kappa.z * rSq * rSq +
		kappa.w * rSq * rSq * rSq);
	return LensCenter + scale * rvector;
}

void main()
{

  int center_x = int(screen_size.x) / 2;

	// The following two variables need to be set per eye
	vec2 LensCenter = gl_FragCoord.x < center_x ? lens_center_left : lens_center_right;
	vec2 ScreenCenter = gl_FragCoord.x < center_x ? screen_center_left : screen_center_right;

	vec2 oTexCoord = gl_FragCoord.xy / screen_size;

	vec2 tc = HmdWarp(oTexCoord, LensCenter);
	if (any(bvec2(clamp(tc,ScreenCenter-vec2(0.25,0.5), ScreenCenter+vec2(0.25,0.5)) - tc)))
	{
	  // We are outside of the warped area
		fragColor = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}

  // double mono video for fake stereo
	//tc.x = gl_FragCoord.x < center_x ? (2.0 * tc.x) : (2.0 * (tc.x - 0.5));
	fragColor = texture2D(texture, tc);
}
