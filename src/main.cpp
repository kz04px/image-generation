#include "defs.hpp"

#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

int window_width;
int window_height;

std::string shader_load(const char* filename)
{
  std::ifstream t(filename);
  std::stringstream buffer;
  buffer << t.rdbuf();
  return buffer.str();
}

bool check_shader_compile_status(GLuint obj)
{
  GLint status;
  glGetShaderiv(obj, GL_COMPILE_STATUS, &status);
  if(status == GL_FALSE)
  {
    GLint length;
    glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &length);
    std::vector<char> log(length);
    glGetShaderInfoLog(obj, length, &length, &log[0]);
    std::cerr << &log[0];
    return false;
  }
  return true;
}

bool check_program_link_status(GLuint obj)
{
  GLint status;
  glGetProgramiv(obj, GL_LINK_STATUS, &status);
  if(status == GL_FALSE)
  {
    GLint length;
    glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &length);
    std::vector<char> log(length);
    glGetProgramInfoLog(obj, length, &length, &log[0]);
    std::cerr << &log[0];
    return false;
  }
  return true;
}

int main()
{
  srand(time(0));
  window_width = 960;
  window_height = 480;

  if(glfwInit() == GL_FALSE)
  {
    std::cerr << "Failed to init GLFW" << std::endl;
    return 1;
  }

  // select opengl version
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  // create a window
  GLFWwindow *window;
  if((window = glfwCreateWindow(window_width, window_height, "Painting Evolution", 0, 0)) == 0)
  {
    std::cerr << "Failed to open window" << std::endl;
    glfwTerminate();
    return 1;
  }

  glfwMakeContextCurrent(window);
  glfwSetWindowSizeCallback(window, glfw_window_size_callback);
  glfwSetCursorPosCallback(window, glfw_cursor_position_callback);
  glfwSetScrollCallback(window, glfw_mouse_scroll_callback);
  glfwSetKeyCallback(window, glfw_keyboard_callback);
  glfwSetMouseButtonCallback(window, glfw_mouse_button_callback);

  // start GLEW extension handler
  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  while((err = glGetError()) != GL_NO_ERROR)
  {
    std::cout << "plsno" << std::endl;
    //getchar();
    //return 0;
  }
  
  #ifndef NDEBUG
  const GLubyte* renderer = glGetString(GL_RENDERER);
  const GLubyte* version = glGetString(GL_VERSION);
  
  std::cout << "Debug info:" << std::endl;
  std::cout << " Date: " << __DATE__ << std::endl;
  std::cout << " Time: " << __TIME__ << std::endl;
  std::cout << " Renderer: " << renderer << std::endl;
  std::cout << " OpenGL version supported: " << version << std::endl; 
  std::cout << " Max textures: " << GL_MAX_TEXTURE_UNITS << std::endl;
  std::cout << std::endl;
  #endif

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  std::string vertex_source = shader_load("shaders\\vertex_shader.glsl");
  std::string fragment_source = shader_load("shaders\\fragment_shader.glsl");

  // program and shader handles
  GLuint shader_program, vertex_shader, fragment_shader;

  // we need these to properly pass the strings
  const char *source;
  int length;

  // create and compile vertex shader
  vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  source = vertex_source.c_str();
  length = vertex_source.size();
  glShaderSource(vertex_shader, 1, &source, &length);
  glCompileShader(vertex_shader);
  if(!check_shader_compile_status(vertex_shader))
  {
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }

  // create and compile fragment shader
  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  source = fragment_source.c_str();
  length = fragment_source.size();
  glShaderSource(fragment_shader, 1, &source, &length);
  glCompileShader(fragment_shader);
  if(!check_shader_compile_status(fragment_shader))
  {
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }

  // create program
  shader_program = glCreateProgram();

  // attach shaders
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);

  // link the program and check for errors
  glLinkProgram(shader_program);
  check_program_link_status(shader_program);

  std::string acceleration_source = shader_load("shaders\\compute_shader.glsl");

  // program and shader handles
  GLuint similarity_program, similarity_shader;

  // create and compile vertex shader
  similarity_shader = glCreateShader(GL_COMPUTE_SHADER);
  source = acceleration_source.c_str();
  length = acceleration_source.size();
  glShaderSource(similarity_shader, 1, &source, &length);
  glCompileShader(similarity_shader);
  if(!check_shader_compile_status(similarity_shader))
  {
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }
  
  // create program
  similarity_program = glCreateProgram();
  
  // attach shaders
  glAttachShader(similarity_program, similarity_shader);
  
  // link the program and check for errors
  glLinkProgram(similarity_program);
  check_program_link_status(similarity_program);
  
  /**** side by side view ****/
  GLuint vao, vbo, cbo, uvbo;
  
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  
  // Vertices
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (char*)0 + 0*sizeof(GLfloat));
  
  // Colours
  glGenBuffers(1, &cbo);
  glBindBuffer(GL_ARRAY_BUFFER, cbo);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (char*)0 + 0*sizeof(GLfloat));
  
  // uvs
  glGenBuffers(1, &uvbo);
  glBindBuffer(GL_ARRAY_BUFFER, uvbo);
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (char*)0 + 0*sizeof(GLfloat));
  /**** side by side view ****/
  
  
  const int num_paintings = 32;
  
  std::vector<GLfloat> scores(num_paintings);
  for(int i = 0; i < num_paintings; ++i)
  {
    scores[i] = 0.0;
  }
  
  // generate vao_compute and scores_bo
  GLuint vao_compute, scores_bo;
  
  glGenVertexArrays(1, &vao_compute);
  glBindVertexArray(vao_compute);
  
  glGenBuffers(1, &scores_bo);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, scores_bo);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat)*num_paintings, &scores[0], GL_DYNAMIC_DRAW); // GL_STATIC_DRAW
  
  // set up generic attrib pointers
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat), (char*)0 + 0*sizeof(GLfloat));
  
	const GLuint ssbos[] = {scores_bo};
  //glBindBuffersBase(GL_SHADER_STORAGE_BUFFER, 0, 1, ssbos);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbos[0]);
  
  // Create target texture
  s_texture input;
  load_bmp(&input, "input.bmp");
  
  GLuint texture = 0;
  glGenTextures(1, &texture);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // GL_LINEAR
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // GL_LINEAR
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, input.w, input.h, 0, GL_RGB, GL_UNSIGNED_BYTE, input.data);
  
  //std::cout << int(input.data[0]) << " " << int(input.data[1]) << " " << int(input.data[2]) << std::endl;
  //std::cout << int(input.data[0])/255.0 << " " << int(input.data[1])/255.0 << " " << int(input.data[2])/255.0 << std::endl;
  //getchar();

  glPointSize(2.0);
  
  GLuint query;
  glGenQueries(1, &query);
  
  s_painting paintings[num_paintings];
  // Init paintings
  for(int p = 0; p < num_paintings; ++p)
  {
    painting_init(&paintings[p], input.w, input.h);
  }

  GLubyte *data = new GLubyte[3*input.w*input.h];

  int gen = 0;
  while(!glfwWindowShouldClose(window))
  {
    glfwPollEvents();
    glBeginQuery(GL_TIME_ELAPSED, query);
    
    // Draw paintings
    for(int p = 0; p < num_paintings; ++p)
    {
      glBindFramebuffer(GL_FRAMEBUFFER, paintings[p].fbo);
      glClearColor(paintings[p].r, paintings[p].g, paintings[p].b, 1.0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      
      // bind the current vao
      glBindVertexArray(vao);
      
      // Vertices
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER, 2 * 3 * paintings[p].num_triangles * sizeof(GLfloat), &paintings[p].positions[0], GL_STATIC_DRAW);
      
      // Colours
      glBindBuffer(GL_ARRAY_BUFFER, cbo);
      glBufferData(GL_ARRAY_BUFFER, 3 * 3 * paintings[p].num_triangles * sizeof(GLfloat), &paintings[p].colours[0], GL_STATIC_DRAW);
      
      // Texture coords
      glBindBuffer(GL_ARRAY_BUFFER, uvbo);
      glBufferData(GL_ARRAY_BUFFER, 2 * 3 * paintings[p].num_triangles * sizeof(GLfloat), &paintings[p].uvs[0], GL_STATIC_DRAW);
      
      // use the shader program
      glUseProgram(shader_program);
      
      // set the uniforms
      glm::mat4 view = glm::ortho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
      glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(view));
      
      // draw
      glDrawArrays(GL_TRIANGLES, 0, 3*paintings[p].num_triangles);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    
    // Reset scores
    for(unsigned int i = 0; i < scores.size(); ++i)
    {
      scores[i] = 0;
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, scores_bo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat)*num_paintings, &scores[0], GL_DYNAMIC_DRAW); // GL_STATIC_DRAW
    

    // Comparisons
    for(int i = 0; i < num_paintings; ++i)
    {
      glUseProgram(similarity_program);
      
      glActiveTexture(GL_TEXTURE0 + texture);
      glBindTexture(GL_TEXTURE_2D, texture);
      
      glActiveTexture(GL_TEXTURE0 + paintings[i].texture_id);
      glBindTexture(GL_TEXTURE_2D, paintings[i].texture_id);
      
      // setup uniforms
      glUniform1i(0, texture);
      glUniform1i(1, paintings[i].texture_id);
      glUniform1i(2, i);

      // Compute
      int groups_x = input.w/16;
      int groups_y = input.h/16;
      glDispatchCompute(groups_x, groups_y, 1);
    }
    glActiveTexture(GL_TEXTURE0);

    // Get scores
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, scores_bo);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, num_paintings*sizeof(scores[0]), &scores[0]);
    

    // Print scores
    for(unsigned int i = 0; i < scores.size() && i < 8; ++i)
    {
      //std::cout << scores[i] << " ";
    }
    //std::cout << std::endl;
    
    
    // Find best painting
    int best_painting = -1;
    float best_score = 1E10;
    for(int p = 0; p < num_paintings; ++p)
    {
      if(scores[p] < best_score)
      {
        best_painting = p;
        best_score = scores[p];
      }
    }
    assert(best_painting != -1);
    assert(best_score < 1E10);
    

    // Create new paintings from best
    for(int p = 0; p < num_paintings; ++p)
    {
      if(p == best_painting) {continue;}
      
      painting_copy(&paintings[p], &paintings[best_painting]);
    }
    
    
    // Mutate paintings
    for(int p = 0; p < num_paintings; ++p)
    {
      //if(p == best_painting) {continue;}
      
      painting_jiggle(&paintings[p]);
    }
    

    // Render target and best paintings
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glBindVertexArray(vao);
    
    glUseProgram(shader_program);
    
    glm::mat4 view = glm::ortho(-1.0, 1.0, 0.0, 1.0, -1.0, 1.0);
    glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(view));
    
    // Render target to the screen
    glBindTexture(GL_TEXTURE_2D, texture);
    GLfloat positions[8] = {-1.0, 0.0,
                            -1.0, 1.0,
                             0.0, 1.0,
                             0.0, 0.0};
    GLfloat colours[12] = {0.0, 0.0, 0.0,
                           0.0, 0.0, 0.0,
                           0.0, 0.0, 0.0,
                           0.0, 0.0, 0.0};
    GLfloat uvs[8] = {0.0, 0.0,
                      0.0, 1.0,
                      1.0, 1.0,
                      1.0, 0.0};
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), &positions, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, cbo);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), &colours, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, uvbo);
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), &uvs, GL_STATIC_DRAW);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    // Render best match to the screen
    glBindTexture(GL_TEXTURE_2D, paintings[best_painting].texture_id);
    positions[0] = 0.0;
    positions[2] = 0.0;
    positions[4] = 1.0;
    positions[6] = 1.0;
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), &positions, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, cbo);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), &colours, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, uvbo);
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), &uvs, GL_STATIC_DRAW);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    glfwSwapBuffers(window);

    // Print scores occasionally
    if(gen%100 == 0)
    {
      std::cout << "Gen " << gen << ": " << best_painting << " - " << 100.0 - 100.0*best_score/(input.w * input.h * 3 * 0.25) << "%" << std::endl;
    }
    
    gen++;
    
    // check for errors
    GLenum error = glGetError();
    if(error != GL_NO_ERROR)
    {
      std::cout << "Error: " << error << std::endl;
    }
    
    // FPS
    glEndQuery(GL_TIME_ELAPSED);
    GLuint64 result;
    glGetQueryObjectui64v(query, GL_QUERY_RESULT, &result);
    std::stringstream tmp;
    tmp << "Painting Evolution: " << int(1e9/result) << " FPS";
    glfwSetWindowTitle(window, tmp.str().c_str());
  }

  delete data;

  // delete the created objects
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &scores_bo);

  glDetachShader(shader_program, vertex_shader);
  glDetachShader(shader_program, fragment_shader);
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);
  glDeleteProgram(shader_program);

  glDetachShader(similarity_program, similarity_shader);
  glDeleteShader(similarity_shader);
  glDeleteProgram(similarity_program);

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
