#include "defs.hpp"

#define MUTATE_RATE_POS 0.01
#define MUTATE_RATE_COL 0.01

int painting_jiggle(s_painting* p)
{
  assert(p != NULL);
  
  p->r += RAND_BETWEEN(-MUTATE_RATE_COL, MUTATE_RATE_COL);
  p->g += RAND_BETWEEN(-MUTATE_RATE_COL, MUTATE_RATE_COL);
  p->b += RAND_BETWEEN(-MUTATE_RATE_COL, MUTATE_RATE_COL);
  
  CLAMP(p->r, 0.0, 1.0);
  CLAMP(p->g, 0.0, 1.0);
  CLAMP(p->b, 0.0, 1.0);
  
  for(int t = 0; t < p->num_triangles; ++t)
  {
    GLfloat dr = RAND_BETWEEN(-MUTATE_RATE_COL, MUTATE_RATE_COL);
    GLfloat dg = RAND_BETWEEN(-MUTATE_RATE_COL, MUTATE_RATE_COL);
    GLfloat db = RAND_BETWEEN(-MUTATE_RATE_COL, MUTATE_RATE_COL);
    
    // Position 1
    p->positions[6*t+0] += RAND_BETWEEN(-MUTATE_RATE_POS, MUTATE_RATE_POS);
    p->positions[6*t+1] += RAND_BETWEEN(-MUTATE_RATE_POS, MUTATE_RATE_POS);
    // Position 2
    p->positions[6*t+2] += RAND_BETWEEN(-MUTATE_RATE_POS, MUTATE_RATE_POS);
    p->positions[6*t+3] += RAND_BETWEEN(-MUTATE_RATE_POS, MUTATE_RATE_POS);
    // Position 3
    p->positions[6*t+4] += RAND_BETWEEN(-MUTATE_RATE_POS, MUTATE_RATE_POS);
    p->positions[6*t+5] += RAND_BETWEEN(-MUTATE_RATE_POS, MUTATE_RATE_POS);
    
    // Colour 1
    p->colours[9*t+0] += dr;
    p->colours[9*t+1] += dg;
    p->colours[9*t+2] += db;
    // Colour 2
    p->colours[9*t+3] += dr;
    p->colours[9*t+4] += dg;
    p->colours[9*t+5] += db;
    // Colour 3
    p->colours[9*t+6] += dr;
    p->colours[9*t+7] += dg;
    p->colours[9*t+8] += db;
  }
  
  return 0;
}

int painting_copy(s_painting* dest, s_painting* src)
{
  assert(dest != NULL);
  assert(src != NULL);

  for(int i = 0; i < src->num_triangles; ++i)
  {
    // Positions
    dest->positions[3*i+0] = src->positions[3*i+0];
    dest->positions[3*i+1] = src->positions[3*i+1];
    dest->positions[3*i+2] = src->positions[3*i+2];
    // Colours
    dest->colours[3*i+0] = src->colours[3*i+0];
    dest->colours[3*i+1] = src->colours[3*i+1];
    dest->colours[3*i+2] = src->colours[3*i+2];
    // uvs
    dest->uvs[3*i+0] = src->uvs[3*i+0];
    dest->uvs[3*i+1] = src->uvs[3*i+1];
    dest->uvs[3*i+2] = src->uvs[3*i+2];
  }

  return 0;
}

int painting_init(s_painting* p, int w, int h)
{
  assert(p != NULL);
  assert(w > 0);
  assert(h > 0);
  
  // create a texture object
  glGenTextures(1, &p->texture_id);
  glActiveTexture(GL_TEXTURE0 + p->texture_id);
  glBindTexture(GL_TEXTURE_2D, p->texture_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // GL_LINEAR
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // GL_LINEAR
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // GL_REPEAT
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // GL_REPEAT
  
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
  //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 960, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
  //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
  
  // create a framebuffer object
  glGenFramebuffers(1, &p->fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, p->fbo);
  
  // attach the texture to FBO color attachment point
  glFramebufferTexture2D(GL_FRAMEBUFFER,        // 1. fbo target: GL_FRAMEBUFFER 
                         GL_COLOR_ATTACHMENT0,  // 2. attachment point
                         GL_TEXTURE_2D,         // 3. tex target: GL_TEXTURE_2D
                         p->texture_id,         // 4. tex ID
                         0);                    // 5. mipmap level: 0(base)
  
  glBindTexture(GL_TEXTURE_2D, 0);
  
  // check FBO status
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if(status != GL_FRAMEBUFFER_COMPLETE)
  {
    return -1;
  }
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  p->num_triangles = 12;
  p->likeness = 0.0;
  
  p->r = RAND_BETWEEN(0.0, 1.0);
  p->g = RAND_BETWEEN(0.0, 1.0);
  p->b = RAND_BETWEEN(0.0, 1.0);
  
  for(int t = 0; t < p->num_triangles; ++t)
  {
    GLfloat r = RAND_BETWEEN(0.0, 1.0);
    GLfloat g = RAND_BETWEEN(0.0, 1.0);
    GLfloat b = RAND_BETWEEN(0.0, 1.0);
    
    // Position 1
    p->positions.push_back(RAND_BETWEEN(0.0, 1.0));
    p->positions.push_back(RAND_BETWEEN(0.0, 1.0));
    // Position 2
    p->positions.push_back(RAND_BETWEEN(0.0, 1.0));
    p->positions.push_back(RAND_BETWEEN(0.0, 1.0));
    // Position 3
    p->positions.push_back(RAND_BETWEEN(0.0, 1.0));
    p->positions.push_back(RAND_BETWEEN(0.0, 1.0));
    
    // Colour 1
    p->colours.push_back(r);
    p->colours.push_back(g);
    p->colours.push_back(b);
    // Colour 2
    p->colours.push_back(r);
    p->colours.push_back(g);
    p->colours.push_back(b);
    // Colour 3
    p->colours.push_back(r);
    p->colours.push_back(g);
    p->colours.push_back(b);
    
    // uv 1
    p->uvs.push_back(-1.0);
    p->uvs.push_back(-1.0);
    // uv 2
    p->uvs.push_back(-1.0);
    p->uvs.push_back(-1.0);
    // uv 3
    p->uvs.push_back(-1.0);
    p->uvs.push_back(-1.0);
  }
  
  return 0;
}