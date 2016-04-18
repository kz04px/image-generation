#version 430

layout(local_size_x=16, local_size_y=16) in;

layout(location = 0) uniform sampler2D targetTexture;
layout(location = 1) uniform sampler2D currentTexture;
layout(location = 2) uniform int id;
layout(std430, binding=0) buffer pblock {float scores[];};

shared float tmp[gl_WorkGroupSize.x * gl_WorkGroupSize.y];

void main()
{
  float xPos = float(gl_WorkGroupID.x*gl_WorkGroupSize.x + gl_LocalInvocationID.x)/(gl_NumWorkGroups.x*gl_WorkGroupSize.x);
  float yPos = float(gl_WorkGroupID.y*gl_WorkGroupSize.y + gl_LocalInvocationID.y)/(gl_NumWorkGroups.y*gl_WorkGroupSize.y);
  vec2 pos = vec2(xPos, yPos);
  
  vec4 dif = texture(targetTexture, pos) - texture(currentTexture, pos);
  float dif_sum = abs(dif.x) + abs(dif.y) + abs(dif.z);

  tmp[gl_WorkGroupSize.x*gl_LocalInvocationID.y + gl_LocalInvocationID.x] = dif_sum;

  barrier();
  //groupMemoryBarrier();

  if(gl_LocalInvocationID.x == 0 && gl_LocalInvocationID.y == 0)
  {
    float total = 0.0;

    for(int i = 0; i < gl_WorkGroupSize.x * gl_WorkGroupSize.y; ++i)
    {
      total += tmp[i];
    }

    scores[gl_WorkGroupID.y*gl_NumWorkGroups.x + gl_WorkGroupID.x] = total;
  }
}
