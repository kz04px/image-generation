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

#define NUM_PAINTINGS 16

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
  int w;
  int h;
  bool tiled_view;
  bool paused;
} s_settings;

// paintings.cpp
int painting_init(s_painting* p, int w, int h);
int painting_jiggle(s_painting* p);
int painting_copy(s_painting* dest, s_painting* src);

// io.cpp
int bmp_load(s_texture* texture, const char *filename);
int bmp_save_n(int n, const char *filename);
std::string shader_load(const char* filename);

// callbacks.cpp
void glfw_window_size_callback(GLFWwindow* window, int width, int height);
void glfw_cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void glfw_mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void glfw_keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

#endif // DEFS_HPP_INCLUDED