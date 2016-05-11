#version 330

in vec3 position;
in vec2 uv;
uniform mat4 mvp;
out vec2 out_uv;

void main()
{
   gl_Position = mvp * vec4(position, 1);
   out_uv = uv;
}

