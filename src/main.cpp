#include "defs.hpp"

#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

int main()
{
  srand(time(0));

  s_settings settings;
  settings.w = 960;
  settings.h = 480;
  settings.tiled_view = true;
  settings.paused = false;

  if(glfwInit() == GL_FALSE)
  {
    std::cerr << "Failed to init GLFW" << std::endl;
    return 1;
  }

  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  GLFWwindow *window;
  if((window = glfwCreateWindow(settings.w, settings.h, "Painting Evolution", 0, 0)) == 0)
  {
    std::cerr << "Failed to open window" << std::endl;
    glfwTerminate();
    return 1;
  }

  glfwSetWindowUserPointer(window, &settings);

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
    std::cout << "glewInit error: " << err << std::endl;
    std::cout << std::endl;
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


  bool r;
  
  GLuint vertex_shader, fragment_shader, compute_shader;

  r = vertex_shader_load(&vertex_shader, "shaders\\vertex_shader.glsl");
  if(r == false)
  {
    glfwDestroyWindow(window);
    glfwTerminate();
  }

  r = fragment_shader_load(&fragment_shader, "shaders\\fragment_shader.glsl");
  if(r == false)
  {
    glfwDestroyWindow(window);
    glfwTerminate();
  }

  r = compute_shader_load(&compute_shader, "shaders\\compute_shader.glsl");
  if(r == false)
  {
    glfwDestroyWindow(window);
    glfwTerminate();
  }


  GLuint shader_program, similarity_program;

  // create program
  shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);
  check_program_link_status(shader_program);
  
  // create program
  similarity_program = glCreateProgram();
  glAttachShader(similarity_program, compute_shader);
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
  

  s_sim sim;
  sim.generation = 0;
  sim.num_paintings = 36;
  sim.tiles_w = 1;
  sim.tiles_h = 1;
  bmp_load(&sim.target, "input.bmp");
  
  s_painting paintings[sim.num_paintings];
  for(int p = 0; p < sim.num_paintings; ++p)
  {
    painting_init(&paintings[p], sim.target.w, sim.target.h);
  }
  
  int num_workgroups = sim.target.w*sim.target.h/16/16;
  std::vector<GLfloat> scores(num_workgroups);
  

  // generate vao_compute and scores_bo
  GLuint vao_compute, scores_bo;
  
  glGenVertexArrays(1, &vao_compute);
  glBindVertexArray(vao_compute);
  
  glGenBuffers(1, &scores_bo);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, scores_bo);
  glBufferData(GL_SHADER_STORAGE_BUFFER, num_workgroups*sizeof(scores[0]), &scores[0], GL_DYNAMIC_DRAW); // GL_STATIC_DRAW
  
  
  GLuint texture = 0;
  glGenTextures(1, &texture);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // GL_LINEAR
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // GL_LINEAR
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, sim.target.w, sim.target.h, 0, GL_RGB, GL_UNSIGNED_BYTE, sim.target.data);

  glPointSize(2.0);

  GLuint query;
  glGenQueries(1, &query);

  int parent1_id = -1;
  int parent2_id = -1;
  float parent1_score = FLT_MAX;
  float parent2_score = FLT_MAX;

  while(!glfwWindowShouldClose(window))
  {
    glBeginQuery(GL_TIME_ELAPSED, query);

    
    if(settings.paused == false)
    {
      glViewport(0, 0, sim.target.w, sim.target.h);
      // Draw paintings
      for(int p = 0; p < sim.num_paintings; ++p)
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
      glViewport(0, 0, settings.w, settings.h);


      // Comparisons
      for(int p = 0; p < sim.num_paintings; ++p)
      {
        glUseProgram(similarity_program);
        
        glActiveTexture(GL_TEXTURE0 + texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        
        glActiveTexture(GL_TEXTURE0 + paintings[p].texture_id);
        glBindTexture(GL_TEXTURE_2D, paintings[p].texture_id);
        
        // setup uniforms
        glUniform1i(0, texture);
        glUniform1i(1, paintings[p].texture_id);
        glUniform1i(2, p);

        // Compute
        int groups_x = sim.target.w/16;
        int groups_y = sim.target.h/16;
        glDispatchCompute(groups_x, groups_y, 1);

        // Get scores
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, scores_bo);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, num_workgroups*sizeof(scores[0]), &scores[0]);

        // Calculate score
        paintings[p].score = 0.0;
        for(int n = 0; n < num_workgroups; ++n)
        {
          paintings[p].score += scores[n];
        }
      }
      glActiveTexture(GL_TEXTURE0);
      
      
      // Find best painting
      parent1_id = -1;
      parent1_score = FLT_MAX;
      parent2_id = -1;
      parent2_score = FLT_MAX;

      for(int p = 0; p < sim.num_paintings; ++p)
      {
        if(paintings[p].score <= parent1_score)
        {
          parent2_id = parent1_id;
          parent2_score = parent1_score;

          parent1_id = p;
          parent1_score = paintings[p].score;
          continue;
        }
        if(paintings[p].score <= parent2_score)
        {
          parent2_id = p;
          parent2_score = paintings[p].score;
        }
      }

      assert(parent1_id != -1);
      assert(parent2_id != -1);
      assert(parent1_id != parent2_id);
      assert(parent1_score >= 0.0);
      assert(parent2_score >= 0.0);
      

      // To be used when a screenshot is taken
      settings.best_painting = paintings[parent1_id].texture_id;


      // Create new paintings from best
      for(int p = 0; p < sim.num_paintings; ++p)
      {
        if(p == parent1_id || p == parent2_id) {continue;}

        paintings_breed(&paintings[p], &paintings[parent1_id], &paintings[parent2_id]);
      }
      
      
      // Mutate paintings
      for(int p = 0; p < sim.num_paintings; ++p)
      {
        if(p == parent1_id || p == parent2_id) {continue;}

        painting_jiggle(&paintings[p]);
      }
      

      // Print scores occasionally
      if(sim.generation%100 == 0)
      {
        std::cout << "Gen " << sim.generation << ": " << parent1_id << " - " << (1.0 - parent1_score/(sim.target.w * sim.target.h * 3))*100.0 << "%" << std::endl;

        /*
        // Print scores
        for(int p = 0; p < sim.num_paintings && p < 8; ++p)
        {
          std::cout << paintings[p].score << " ";
        }
        std::cout << std::endl;
        std::cout << std::endl;
        */
      }
      
      sim.generation++;
    }

    // Render target and best paintings
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0, 0.0, 0.0, 1.0);
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
    
    if(settings.tiled_view == true)
    {
      int x_num = ceil(sqrt(sim.num_paintings));
      int y_num = ceil(sqrt(sim.num_paintings));

      if(x_num*(y_num-1) >= sim.num_paintings)
      {
        y_num -= 1;
      }

      float width  = 1.0/x_num;
      float height = 1.0/y_num;
      
      for(int y = 0; y < y_num; ++y)
      {
        for(int x = 0; x < x_num; ++x)
        {
          if(y*x_num + x >= sim.num_paintings) {break;}
          
          glBindTexture(GL_TEXTURE_2D, paintings[y*y_num + x].texture_id);
          positions[0] = x*width;     positions[1] = y*height;
          positions[2] = x*width;     positions[3] = (y+1)*height;
          positions[4] = (x+1)*width; positions[5] = (y+1)*height;
          positions[6] = (x+1)*width; positions[7] = y*height;
          glBindBuffer(GL_ARRAY_BUFFER, vbo);
          glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), &positions, GL_STATIC_DRAW);
          glBindBuffer(GL_ARRAY_BUFFER, cbo);
          glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), &colours, GL_STATIC_DRAW);
          glBindBuffer(GL_ARRAY_BUFFER, uvbo);
          glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), &uvs, GL_STATIC_DRAW);
          glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        }
      }
    }
    else
    {
      glBindTexture(GL_TEXTURE_2D, paintings[parent1_id].texture_id);
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
    }

    glfwSwapBuffers(window);
    
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

    glfwPollEvents();
  }

  // delete the created objects
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &scores_bo);

  glDetachShader(shader_program, vertex_shader);
  glDetachShader(shader_program, fragment_shader);
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);
  glDeleteProgram(shader_program);

  glDetachShader(similarity_program, compute_shader);
  glDeleteShader(compute_shader);
  glDeleteProgram(similarity_program);

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
