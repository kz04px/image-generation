#version 430

/*
1280 - GL_INVALID_ENUM
1281 - GL_INVALID_VALUE
1282 - GL_INVALID_OPERATION
*/

layout(local_size_x=256) in;

layout(location = 0) uniform sampler2D targetTexture;
layout(location = 1) uniform sampler2D currentTexture;
layout(location = 2) uniform int id;
layout(std430, binding=0) buffer pblock {float scores[];};

shared float tmp[gl_WorkGroupSize.x * gl_WorkGroupSize.y];

void main()
{
  scores[0] = texture(targetTexture, vec2(0, 0)).r + 1.5;
  scores[1] = texture(targetTexture, vec2(0, 0)).g;
  scores[2] = texture(targetTexture, vec2(0, 0)).b;

  scores[3] = -1.0;

  /*
  uint startX = gl_WorkGroupID.x*16;
  uint startY = gl_WorkGroupID.y*16;
  
  vec3 dif_sum = vec3(0, 0, 0);
  
  for(uint x = 0; x < 16; ++x)
  {
    for(uint y = 0; y < 16; ++y)
    {
      vec3 dif = texture(targetTexture, vec2(startX + x, startY + y)).rgb - texture(currentTexture, vec2(startX + x, startY + y)).rgb;
      dif_sum += abs(dif.x) + abs(dif.y) + abs(dif.z);
    }
  }

  tmp[gl_WorkGroupID.y*16 + gl_WorkGroupID.x] = abs(dif_sum.x) + abs(dif_sum.y) + abs(dif_sum.z);

  //barrier();
  groupMemoryBarrier();

  if(gl_WorkGroupID.x == 0 && gl_WorkGroupID.y == 0)
  {
    scores[id] = 0.0;

    for(int i = 0; i < gl_WorkGroupSize.x * gl_WorkGroupSize.y; ++i)
    {
      scores[id] += abs(dif_sum.x) + abs(dif_sum.y) + abs(dif_sum.z);
    }

    scores[0] = texture(targetTexture, vec2(0, 0)).r;
    scores[1] = texture(targetTexture, vec2(0, 0)).g;
    scores[2] = texture(targetTexture, vec2(0, 0)).b;

    scores[3] = -1.0;
  }
  */



  //scores[id] += abs(dif_sum.x) + abs(dif_sum.y) + abs(dif_sum.z);
  
  //barrier();

  /*
  if(gl_WorkGroupID.x > scores[0])
    scores[0] = gl_WorkGroupID.x;
  if(gl_WorkGroupID.y > scores[1])
    scores[1] = gl_WorkGroupID.y;
  if(gl_LocalInvocationIndex > scores[2])
    scores[2] = gl_LocalInvocationIndex;
  */

  /*
  scores[0] = texture(targetTexture, vec2(0, 0)).r;
  scores[1] = texture(targetTexture, vec2(0, 0)).g;
  scores[2] = texture(targetTexture, vec2(0, 0)).b;
  */

  /*
  vec3 test = texture(targetTexture, vec2(0, 0)).rgb - texture(currentTexture, vec2(0, 0)).rgb;
  scores[id] = abs(test.x) + abs(test.y) + abs(test.z);
  */
}
