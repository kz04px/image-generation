#ifndef DEFS_HPP_INCLUDED
#define DEFS_HPP_INCLUDED

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <assert.h>

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <time.h>
#include <float.h>

#define RAND_BETWEEN(a, b) ((float)rand()/RAND_MAX*(b-a) + a)
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(x, a, b) ((x) < (a) ? (x)=(a) : ((x) > (b) ? (x)=(b) : 0))

#define MODE_AUTO   0
#define MODE_MANUAL 1

#define TARGET_PERCENTAGE 0.975
#define MIN_GEN 1000
#define MAX_GEN 10000
#define GRAYSCALE
//#define BACKGROUNDS

typedef struct
{
  int w;
  int h;
  unsigned char *data;
} s_texture;

typedef struct
{
  GLuint fbo;
  GLuint texture_id;
  
  int generation;

  GLfloat r;
  GLfloat g;
  GLfloat b;
  
  int num_triangles;
  float score;
  std::vector<GLfloat> positions;
  std::vector<GLfloat> colours;
  std::vector<GLfloat> uvs;
} s_painting;

typedef struct
{
  int grid_pos;
  int grid_w;
  int grid_h;
  int tile_w;
  int tile_h;

  GLuint result_id;
  GLuint result_fbo;

  GLuint target_id;
  s_texture target;

  std::vector<s_painting> grid_paintings;
  std::vector<s_painting> paintings;
} s_sim;

typedef struct
{
  int mode;
  int w;
  int h;
  int best_painting;
  bool tiled_view;
  bool paused;
  s_sim *sim;
} s_settings;

// paintings.cpp
int painting_init(s_painting* p, int w, int h);
int painting_jiggle(s_painting* p);
int painting_copy(s_painting* dest, s_painting* src);
int paintings_breed(s_painting* child, s_painting* parent1, s_painting* parent2);
int painting_randomise(s_painting* p);
int paintings_save(s_sim *sim, const char *filename);
int paintings_load(s_sim *sim, const char *filename);

// other.cpp
bool check_shader_compile_status(GLuint obj);
bool check_program_link_status(GLuint obj);

// io.cpp
int bmp_load(s_texture* texture, const char *filename);
int bmp_save_n(int n, const char *filename);
bool vertex_shader_load(GLuint* vertex_shader, const char* filename);
bool fragment_shader_load(GLuint* fragment_shader, const char* filename);
bool compute_shader_load(GLuint* compute_shader, const char* filename);

// callbacks.cpp
void glfw_window_size_callback(GLFWwindow* window, int width, int height);
void glfw_cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void glfw_mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void glfw_keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

#endif // DEFS_HPP_INCLUDED