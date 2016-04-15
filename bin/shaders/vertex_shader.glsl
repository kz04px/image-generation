#version 430

layout(location = 0) uniform mat4 view;
layout(location = 0) in vec2 v_position;
layout(location = 1) in vec3 v_colour;
layout(location = 2) in vec2 v_uv;
layout(location = 0) out vec4 colour;
layout(location = 1) out vec2 uv;

void main()
{
  gl_Position = view*vec4(v_position, 0.0, 1.0);
  colour = vec4(v_colour, 0.5);
  uv = v_uv;
}