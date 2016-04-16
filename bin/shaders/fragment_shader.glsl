#version 430

uniform sampler2D my_texture;

layout(location = 0) in vec4 colour;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 FragColor;

void main()
{
  if(uv.x < 0.0 || uv.y < 0.0)
  {
    FragColor = colour;
  }
  else
  {
    FragColor = vec4(texture(my_texture, uv).rgb, 1.0);
  }
}