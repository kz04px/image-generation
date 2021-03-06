#include "defs.hpp"
#include <string.h>

#define MUTATE_RATE_POS 0.01
#define MUTATE_RATE_COL 0.01

int paintings_load(s_sim *sim, const char *filename)
{
  assert(sim != NULL);
  assert(filename != NULL);

  FILE *file = fopen(filename, "r");
  if(file == NULL) {return -1;}

  unsigned int p = 0;
  int t = 0;

  char line[512];
  while(fgets(line, sizeof(line), file))
  {
    if(line[0] == '#') {continue;}

    char *setting = strtok(line, " ");
    char *value = strtok(NULL, " \n\0");

    if(strncmp(setting, "Painting:", 9) == 0)
    {
      p = atoi(value);
      if(p >= sim->grid_paintings.size()) {return -2;}
      t = 0;
    }
    else if(strncmp(setting, "Score:", 6) == 0)
    {
      sim->grid_paintings[p].score = atof(value);
    }
    else if(strncmp(setting, "Rate:", 5) == 0)
    {
      sim->grid_paintings[p].score_rate = atof(value);
    }
    else if(strncmp(setting, "Generation:", 11) == 0)
    {
      sim->grid_paintings[p].generation = atoi(value);
    }
    else if(strncmp(setting, "r:", 2) == 0)
    {
      sim->grid_paintings[p].r = atof(value);
    }
    else if(strncmp(setting, "g:", 2) == 0)
    {
      sim->grid_paintings[p].g = atof(value);
    }
    else if(strncmp(setting, "b:", 2) == 0)
    {
      sim->grid_paintings[p].b = atof(value);
    }
    else if(strncmp(setting, "TRI:", 4) == 0)
    {
      sim->grid_paintings[p].positions[6*t+0] = atof(value);
      value = strtok(NULL, " \n\0");
      sim->grid_paintings[p].positions[6*t+1] = atof(value);
      value = strtok(NULL, " \n\0");
      sim->grid_paintings[p].positions[6*t+2] = atof(value);
      value = strtok(NULL, " \n\0");
      sim->grid_paintings[p].positions[6*t+3] = atof(value);
      value = strtok(NULL, " \n\0");
      sim->grid_paintings[p].positions[6*t+4] = atof(value);
      value = strtok(NULL, " \n\0");
      sim->grid_paintings[p].positions[6*t+5] = atof(value);
      value = strtok(NULL, " \n\0");

      float r = atof(value);
      value = strtok(NULL, " \n\0");
      float g = atof(value);
      value = strtok(NULL, " \n\0");
      float b = atof(value);

      sim->grid_paintings[p].colours[9*t+0] = r;
      sim->grid_paintings[p].colours[9*t+1] = g;
      sim->grid_paintings[p].colours[9*t+2] = b;
      sim->grid_paintings[p].colours[9*t+3] = r;
      sim->grid_paintings[p].colours[9*t+4] = g;
      sim->grid_paintings[p].colours[9*t+5] = b;
      sim->grid_paintings[p].colours[9*t+6] = r;
      sim->grid_paintings[p].colours[9*t+7] = g;
      sim->grid_paintings[p].colours[9*t+8] = b;

      t++;
    }
  }

  fclose(file);
  return 0;
}

int paintings_save(s_sim *sim, const char *filename)
{
  assert(sim != NULL);
  assert(filename != NULL);

  FILE* file = fopen(filename, "w");
  if(file == NULL) {return -1;}

  for(unsigned int p = 0; p < sim->grid_paintings.size(); ++p)
  {
    fprintf(file, "Painting: %i\n", p);
    fprintf(file, "Score: %f\n", sim->grid_paintings[p].score);
    fprintf(file, "Rate: %f\n", sim->grid_paintings[p].score_rate);
    fprintf(file, "Generation: %i\n", sim->grid_paintings[p].generation);
    fprintf(file, "r: %f\n", sim->grid_paintings[p].r);
    fprintf(file, "g: %f\n", sim->grid_paintings[p].g);
    fprintf(file, "b: %f\n", sim->grid_paintings[p].b);
    for(int t = 0; t < sim->grid_paintings[p].num_triangles; ++t)
    {
      fprintf(file, "TRI: %f %f %f %f %f %f %f %f %f\n", sim->grid_paintings[p].positions[6*t+0],
                                                         sim->grid_paintings[p].positions[6*t+1],
                                                         sim->grid_paintings[p].positions[6*t+2],
                                                         sim->grid_paintings[p].positions[6*t+3],
                                                         sim->grid_paintings[p].positions[6*t+4],
                                                         sim->grid_paintings[p].positions[6*t+5],
                                                         sim->grid_paintings[p].colours[9*t+0],
                                                         sim->grid_paintings[p].colours[9*t+1],
                                                         sim->grid_paintings[p].colours[9*t+2]);
    }
  }

  fclose(file);
  return 0;
}

int painting_randomise(s_painting* p)
{
  assert(p != NULL);

  #ifdef BACKGROUNDS
  p->r = RAND_BETWEEN(0.0, 1.0);
  p->g = RAND_BETWEEN(0.0, 1.0);
  p->b = RAND_BETWEEN(0.0, 1.0);
  #endif

  p->generation = 0;
  p->score = 0.0;

  for(int t = 0; t < p->num_triangles; ++t)
  {
    // Position 1
    p->positions[6*t+0] = RAND_BETWEEN(0.0, 1.0);
    p->positions[6*t+1] = RAND_BETWEEN(0.0, 1.0);
    // Position 2
    p->positions[6*t+2] = RAND_BETWEEN(0.0, 1.0);
    p->positions[6*t+3] = RAND_BETWEEN(0.0, 1.0);
    // Position 3
    p->positions[6*t+4] = RAND_BETWEEN(0.0, 1.0);
    p->positions[6*t+5] = RAND_BETWEEN(0.0, 1.0);

    GLfloat dr = RAND_BETWEEN(0.0, 1.0);
    GLfloat dg = RAND_BETWEEN(0.0, 1.0);
    GLfloat db = RAND_BETWEEN(0.0, 1.0);

    #ifdef GRAYSCALE
    dg = dr;
    db = dr;
    #endif
    
    // Colour 1
    p->colours[9*t+0] = dr;
    p->colours[9*t+1] = dg;
    p->colours[9*t+2] = db;
    // Colour 2
    p->colours[9*t+3] = dr;
    p->colours[9*t+4] = dg;
    p->colours[9*t+5] = db;
    // Colour 3
    p->colours[9*t+6] = dr;
    p->colours[9*t+7] = dg;
    p->colours[9*t+8] = db;
  }

  return 0;
}

int painting_jiggle(s_painting* p)
{
  assert(p != NULL);

  /*
   * 1/10 - Full mutate
   * 2/10 - Jiggle
   */

  #ifdef BACKGROUNDS
  p->r += RAND_BETWEEN(-MUTATE_RATE_COL, MUTATE_RATE_COL);
  p->g += RAND_BETWEEN(-MUTATE_RATE_COL, MUTATE_RATE_COL);
  p->b += RAND_BETWEEN(-MUTATE_RATE_COL, MUTATE_RATE_COL);
  
  CLAMP(p->r, 0.0, 1.0);
  CLAMP(p->g, 0.0, 1.0);
  CLAMP(p->b, 0.0, 1.0);
  #endif

  int t = rand()%p->num_triangles;

  if(rand()%10 == 0)
  {
    // Position 1
    p->positions[6*t+0] = RAND_BETWEEN(0.0, 1.0);
    p->positions[6*t+1] = RAND_BETWEEN(0.0, 1.0);
    // Position 2
    p->positions[6*t+2] = RAND_BETWEEN(0.0, 1.0);
    p->positions[6*t+3] = RAND_BETWEEN(0.0, 1.0);
    // Position 3
    p->positions[6*t+4] = RAND_BETWEEN(0.0, 1.0);
    p->positions[6*t+5] = RAND_BETWEEN(0.0, 1.0);

    CLAMP(p->positions[6*t+2], p->positions[6*t+0] - 0.1, p->positions[6*t+0] + 0.1);
    CLAMP(p->positions[6*t+3], p->positions[6*t+1] - 0.1, p->positions[6*t+1] + 0.1);
    CLAMP(p->positions[6*t+4], p->positions[6*t+0] - 0.1, p->positions[6*t+0] + 0.1);
    CLAMP(p->positions[6*t+5], p->positions[6*t+1] - 0.1, p->positions[6*t+1] + 0.1);

    GLfloat dr = RAND_BETWEEN(0.0, 1.0);
    GLfloat dg = RAND_BETWEEN(0.0, 1.0);
    GLfloat db = RAND_BETWEEN(0.0, 1.0);

    #ifdef GRAYSCALE
    dg = dr;
    db = dr;
    #endif
    
    // Colour 1
    p->colours[9*t+0] = dr;
    p->colours[9*t+1] = dg;
    p->colours[9*t+2] = db;
    // Colour 2
    p->colours[9*t+3] = dr;
    p->colours[9*t+4] = dg;
    p->colours[9*t+5] = db;
    // Colour 3
    p->colours[9*t+6] = dr;
    p->colours[9*t+7] = dg;
    p->colours[9*t+8] = db;
  }
  //else if(rand()%3 == 0)
  else
  {
    GLfloat dr = RAND_BETWEEN(-MUTATE_RATE_COL, MUTATE_RATE_COL);
    GLfloat dg = RAND_BETWEEN(-MUTATE_RATE_COL, MUTATE_RATE_COL);
    GLfloat db = RAND_BETWEEN(-MUTATE_RATE_COL, MUTATE_RATE_COL);

    #ifdef GRAYSCALE
    dg = dr;
    db = dr;
    #endif
    
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

    /*
    CLAMP(p->positions[6*t+0], 0.0, 1.0);
    CLAMP(p->positions[6*t+1], 0.0, 1.0);
    CLAMP(p->positions[6*t+2], p->positions[6*t+0] - 0.1, p->positions[6*t+0] + 0.1);
    CLAMP(p->positions[6*t+3], p->positions[6*t+1] - 0.1, p->positions[6*t+1] + 0.1);
    CLAMP(p->positions[6*t+4], p->positions[6*t+0] - 0.1, p->positions[6*t+0] + 0.1);
    CLAMP(p->positions[6*t+5], p->positions[6*t+1] - 0.1, p->positions[6*t+1] + 0.1);
    */

    CLAMP(p->positions[6*t+0], 0.0, 1.0);
    CLAMP(p->positions[6*t+1], 0.0, 1.0);
    CLAMP(p->positions[6*t+2], 0.0, 1.0);
    CLAMP(p->positions[6*t+3], 0.0, 1.0);
    CLAMP(p->positions[6*t+4], 0.0, 1.0);
    CLAMP(p->positions[6*t+5], 0.0, 1.0);

    CLAMP(p->colours[9*t+0], 0.0, 1.0);
    CLAMP(p->colours[9*t+1], 0.0, 1.0);
    CLAMP(p->colours[9*t+2], 0.0, 1.0);
    CLAMP(p->colours[9*t+3], 0.0, 1.0);
    CLAMP(p->colours[9*t+4], 0.0, 1.0);
    CLAMP(p->colours[9*t+5], 0.0, 1.0);
    CLAMP(p->colours[9*t+6], 0.0, 1.0);
    CLAMP(p->colours[9*t+7], 0.0, 1.0);
    CLAMP(p->colours[9*t+8], 0.0, 1.0);
  }
  
  return 0;
}

int paintings_breed(s_painting* child, s_painting* parent1, s_painting* parent2)
{
  assert(child != NULL);
  assert(parent1 != NULL);
  assert(parent2 != NULL);

  if(rand()%2 == 0)
  {
    child->r = parent1->r;
    child->g = parent1->g;
    child->b = parent1->b;
  }
  else
  {
    child->r = parent2->r;
    child->g = parent2->g;
    child->b = parent2->b;
  }

  for(int i = 0; i < parent1->num_triangles; ++i)
  {
    if(rand()%2 == 0)
    {
      // Positions
      child->positions[6*i+0] = parent1->positions[6*i+0];
      child->positions[6*i+1] = parent1->positions[6*i+1];
      child->positions[6*i+2] = parent1->positions[6*i+2];
      child->positions[6*i+3] = parent1->positions[6*i+3];
      child->positions[6*i+4] = parent1->positions[6*i+4];
      child->positions[6*i+5] = parent1->positions[6*i+5];
      // Colours
      child->colours[9*i+0] = parent1->colours[9*i+0];
      child->colours[9*i+1] = parent1->colours[9*i+1];
      child->colours[9*i+2] = parent1->colours[9*i+2];
      child->colours[9*i+3] = parent1->colours[9*i+3];
      child->colours[9*i+4] = parent1->colours[9*i+4];
      child->colours[9*i+5] = parent1->colours[9*i+5];
      child->colours[9*i+6] = parent1->colours[9*i+6];
      child->colours[9*i+7] = parent1->colours[9*i+7];
      child->colours[9*i+8] = parent1->colours[9*i+8];
      // uvs
      child->uvs[6*i+0] = parent1->uvs[6*i+0];
      child->uvs[6*i+1] = parent1->uvs[6*i+1];
      child->uvs[6*i+2] = parent1->uvs[6*i+2];
      child->uvs[6*i+3] = parent1->uvs[6*i+3];
      child->uvs[6*i+4] = parent1->uvs[6*i+4];
      child->uvs[6*i+5] = parent1->uvs[6*i+5];
    }
    else
    {
      // Positions
      child->positions[6*i+0] = parent2->positions[6*i+0];
      child->positions[6*i+1] = parent2->positions[6*i+1];
      child->positions[6*i+2] = parent2->positions[6*i+2];
      child->positions[6*i+3] = parent2->positions[6*i+3];
      child->positions[6*i+4] = parent2->positions[6*i+4];
      child->positions[6*i+5] = parent2->positions[6*i+5];
      // Colours
      child->colours[9*i+0] = parent2->colours[9*i+0];
      child->colours[9*i+1] = parent2->colours[9*i+1];
      child->colours[9*i+2] = parent2->colours[9*i+2];
      child->colours[9*i+3] = parent2->colours[9*i+3];
      child->colours[9*i+4] = parent2->colours[9*i+4];
      child->colours[9*i+5] = parent2->colours[9*i+5];
      child->colours[9*i+6] = parent2->colours[9*i+6];
      child->colours[9*i+7] = parent2->colours[9*i+7];
      child->colours[9*i+8] = parent2->colours[9*i+8];
      // uvs
      child->uvs[6*i+0] = parent2->uvs[6*i+0];
      child->uvs[6*i+1] = parent2->uvs[6*i+1];
      child->uvs[6*i+2] = parent2->uvs[6*i+2];
      child->uvs[6*i+3] = parent2->uvs[6*i+3];
      child->uvs[6*i+4] = parent2->uvs[6*i+4];
      child->uvs[6*i+5] = parent2->uvs[6*i+5];
    }
  }

  return 0;
}

int painting_copy(s_painting* dest, s_painting* src)
{
  assert(dest != NULL);
  assert(src != NULL);

  dest->num_triangles = src->num_triangles;
  dest->generation = src->generation;
  dest->score = src->score;
  dest->score_rate = src->score_rate;

  dest->r = src->r;
  dest->g = src->g;
  dest->b = src->b;

  for(int i = 0; i < src->num_triangles; ++i)
  {
    // Positions
    dest->positions[6*i+0] = src->positions[6*i+0];
    dest->positions[6*i+1] = src->positions[6*i+1];
    dest->positions[6*i+2] = src->positions[6*i+2];
    dest->positions[6*i+3] = src->positions[6*i+3];
    dest->positions[6*i+4] = src->positions[6*i+4];
    dest->positions[6*i+5] = src->positions[6*i+5];
    // Colours
    dest->colours[9*i+0] = src->colours[9*i+0];
    dest->colours[9*i+1] = src->colours[9*i+1];
    dest->colours[9*i+2] = src->colours[9*i+2];
    dest->colours[9*i+3] = src->colours[9*i+3];
    dest->colours[9*i+4] = src->colours[9*i+4];
    dest->colours[9*i+5] = src->colours[9*i+5];
    dest->colours[9*i+6] = src->colours[9*i+6];
    dest->colours[9*i+7] = src->colours[9*i+7];
    dest->colours[9*i+8] = src->colours[9*i+8];
    // uvs
    dest->uvs[6*i+0] = src->uvs[6*i+0];
    dest->uvs[6*i+1] = src->uvs[6*i+1];
    dest->uvs[6*i+2] = src->uvs[6*i+2];
    dest->uvs[6*i+3] = src->uvs[6*i+3];
    dest->uvs[6*i+4] = src->uvs[6*i+4];
    dest->uvs[6*i+5] = src->uvs[6*i+5];
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
  
  p->generation = 0;
  p->num_triangles = 64;
  p->score = 0.0;
  p->score_rate = FLT_MAX;
  
  #ifdef BACKGROUNDS
  p->r = RAND_BETWEEN(0.0, 1.0);
  p->g = RAND_BETWEEN(0.0, 1.0);
  p->b = RAND_BETWEEN(0.0, 1.0);

  #ifdef GRAYSCALE
  p->g = p->r;
  p->b = p->r;
  #endif

  #else

  #ifdef GRAYSCALE
  p->r = 0.5;
  p->g = 0.5;
  p->b = 0.5;
  #else
  p->r = 1.0;
  p->g = 1.0;
  p->b = 1.0;
  #endif

  #endif

  for(int t = 0; t < p->num_triangles; ++t)
  {
    GLfloat r = RAND_BETWEEN(0.0, 1.0);
    GLfloat g = RAND_BETWEEN(0.0, 1.0);
    GLfloat b = RAND_BETWEEN(0.0, 1.0);

    #ifdef GRAYSCALE
    g = r;
    b = r;
    #endif

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