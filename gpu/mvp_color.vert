#version 330

in vec4 position;
in vec3 color;
uniform mat4 mvp;
out vec3 out_color;

void main()
{
   gl_Position = mvp * position;
   out_color = color;
}

