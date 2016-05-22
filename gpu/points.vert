#version 330

in vec3 position;
in vec2 uv;
uniform mat4 mvp;
out vec2 out_uv;
uniform sampler2D texture;

void main()
{

    vec2 ndcPos = position.xy;
    vec2 in_xy = vec2(0.5) + 0.5*(ndcPos);
    out_uv = in_xy;

    // This is where the magic should happen - sample the depth texture
    // directly from the vertex shader and set each point's location
    // according to its nearest sampled depth value.
    vec4 texel = textureLod(texture, in_xy, 0.0);
    float depthValue = texel.r;

    vec3 pos = position;
    pos.z = depthValue;
    // Push all points of unknown depth away where they shouldn't be visible.
    if (depthValue == 0.0)
    {
        pos = vec3(9999.0);
    }
   gl_Position = mvp * vec4(pos, 1.0);


   //gl_Position = mvp * vec4(position, 1);
   //out_uv = uv;
}

