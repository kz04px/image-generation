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
  settings.mode = MODE_AUTO;
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
  bmp_load(&sim.target, "input.bmp");
  
  sim.grid_pos = 0;
  sim.grid_w = 8;
  sim.grid_h = 8;
  sim.tile_w = sim.target.w/sim.grid_w;
  sim.tile_h = sim.target.h/sim.grid_h;

  /*
  if(sim.grid_w*sim.grid_h >= GL_MAX_TEXTURE_UNITS)
  {
    std::cout << "Uh oh" << std::endl;
    getchar();
  }
  */

  settings.sim = &sim;

  assert(sim.target.w%16 == 0);
  assert(sim.target.h%16 == 0);
  assert(sim.tile_w%16 == 0);
  assert(sim.tile_h%16 == 0);
  
  s_painting temp;
  for(int p = 0; p < 64; ++p)
  {
    sim.paintings.push_back(temp);
    painting_init(&sim.paintings[p], sim.tile_w, sim.tile_h);
  }
  for(int p = 0; p < sim.grid_w*sim.grid_h; ++p)
  {
    sim.grid_paintings.push_back(temp);
    painting_init(&sim.grid_paintings[p], sim.tile_w, sim.tile_h);
  }
  
  int num_workgroups = sim.tile_w*sim.tile_h/16/16;
  std::vector<GLfloat> scores(num_workgroups);
  

  // generate vao_compute and scores_bo
  GLuint vao_compute, scores_bo;
  
  glGenVertexArrays(1, &vao_compute);
  glBindVertexArray(vao_compute);
  
  glGenBuffers(1, &scores_bo);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, scores_bo);
  glBufferData(GL_SHADER_STORAGE_BUFFER, num_workgroups*sizeof(scores[0]), &scores[0], GL_DYNAMIC_DRAW); // GL_STATIC_DRAW
  
  
  glGenTextures(1, &sim.target_id);
  glBindTexture(GL_TEXTURE_2D, sim.target_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // GL_LINEAR
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // GL_LINEAR
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, sim.target.w, sim.target.h, 0, GL_RGB, GL_UNSIGNED_BYTE, sim.target.data);

  glGenTextures(1, &sim.result_id);
  glBindTexture(GL_TEXTURE_2D, sim.result_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // GL_LINEAR
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // GL_LINEAR
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, 4*sim.target.w, 4*sim.target.h, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

  std::vector<GLubyte> emptyData(sim.target.w * sim.target.h * 3 * 16, 0);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4*sim.target.w, 4*sim.target.h, GL_RGB, GL_UNSIGNED_BYTE, &emptyData[0]);
  
  
  GLuint highres_texture_id = 0;
  glGenTextures(1, &highres_texture_id);
  glBindTexture(GL_TEXTURE_2D, highres_texture_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // GL_LINEAR
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // GL_LINEAR
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, 4*sim.tile_w, 4*sim.tile_h, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

  // create a framebuffer object
  GLuint highres_fbo = 0;
  glGenFramebuffers(1, &highres_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, highres_fbo);
  
  // attach the texture to FBO color attachment point
  glFramebufferTexture2D(GL_FRAMEBUFFER,        // 1. fbo target: GL_FRAMEBUFFER 
                         GL_COLOR_ATTACHMENT0,  // 2. attachment point
                         GL_TEXTURE_2D,         // 3. tex target: GL_TEXTURE_2D
                         highres_texture_id,    // 4. tex ID
                         0);                    // 5. mipmap level: 0(base)

  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  glBindTexture(GL_TEXTURE_2D, 0);
  
  // check FBO status
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if(status != GL_FRAMEBUFFER_COMPLETE)
  {
    std::cout << "Framebuffer error:" << status<< std::endl;
    return -1;
  }

  GLuint texture_slice = 0;
  glGenTextures(1, &texture_slice);
  glActiveTexture(GL_TEXTURE0 + texture_slice);
  glBindTexture(GL_TEXTURE_2D, texture_slice);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // GL_LINEAR
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // GL_LINEAR
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, sim.tile_w, sim.tile_h, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

  glPointSize(2.0);

  GLuint query;
  glGenQueries(1, &query);


  GLubyte *data = new GLubyte[3*sim.tile_w*sim.tile_h*16];
  //GLubyte data[3*sim.tile_w*sim.tile_h*16];

  unsigned int parent1_id = 0;
  unsigned int parent2_id = 0;
  float parent1_score = FLT_MIN;
  float parent2_score = FLT_MIN;

  int x_offset = 0;
  int y_offset = 0;
  int tile_x = 0;
  int tile_y = 0;
  int last_grid_pos = -1;
  int auto_generation = 0;
  float start_score = 0.0;

  while(!glfwWindowShouldClose(window))
  {
    glBeginQuery(GL_TIME_ELAPSED, query);

    if(settings.paused == false)
    {
      // Auto mode will spend a set number of generations per grid position and then move to the new lowest scoring
      if(settings.mode == MODE_AUTO && auto_generation >= AUTO_GENERATIONS)
      {
        // Set score rate
        sim.grid_paintings[sim.grid_pos].score_rate = (sim.grid_paintings[sim.grid_pos].score - start_score)/AUTO_GENERATIONS;

        assert(sim.grid_paintings[sim.grid_pos].score_rate >= 0.0);
        assert(sim.grid_paintings[sim.grid_pos].score_rate <= 1.0/AUTO_GENERATIONS);

        float fastest = FLT_MIN;
        for(unsigned int p = 0; p < sim.grid_paintings.size(); ++p)
        {
          if(sim.grid_paintings[p].score_rate > fastest)
          {
            sim.grid_pos = p;
            fastest = sim.grid_paintings[p].score_rate;
          }
        }

        start_score = sim.grid_paintings[sim.grid_pos].score;
        auto_generation = 0;
      }

      // Next tile
      if(last_grid_pos != sim.grid_pos)
      {
        if(last_grid_pos != -1)
        {
          // Update results texture - do all grid paintings because new ones might've been loaded in
          for(unsigned int p = 0; p < sim.grid_paintings.size(); ++p)
          {
            if(sim.grid_paintings[p].generation == 0) {continue;}

            tile_x = p%sim.grid_w;
            tile_y = p/sim.grid_w;
            
            x_offset = tile_x * sim.tile_w;
            y_offset = tile_y * sim.tile_h;
        
            // Save current best paiting to the results
            glViewport(0, 0, 4*sim.tile_w, 4*sim.tile_h);

            // Redraw
            glBindFramebuffer(GL_FRAMEBUFFER, highres_fbo);
            glClearColor(sim.grid_paintings[p].r, sim.grid_paintings[p].g, sim.grid_paintings[p].b, 1.0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            // Bind the current vao
            glBindVertexArray(vao);
            
            // Vertices
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, 2 * 3 * sim.grid_paintings[p].num_triangles * sizeof(GLfloat), &sim.grid_paintings[p].positions[0], GL_STATIC_DRAW);
            
            // Colours
            glBindBuffer(GL_ARRAY_BUFFER, cbo);
            glBufferData(GL_ARRAY_BUFFER, 3 * 3 * sim.grid_paintings[p].num_triangles * sizeof(GLfloat), &sim.grid_paintings[p].colours[0], GL_STATIC_DRAW);
            
            // Texture coords
            glBindBuffer(GL_ARRAY_BUFFER, uvbo);
            glBufferData(GL_ARRAY_BUFFER, 2 * 3 * sim.grid_paintings[p].num_triangles * sizeof(GLfloat), &sim.grid_paintings[p].uvs[0], GL_STATIC_DRAW);
            
            glUseProgram(shader_program);
            
            // Set the uniforms
            glm::mat4 view = glm::ortho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(view));
            
            // Draw
            glDrawArrays(GL_TRIANGLES, 0, 3*sim.grid_paintings[p].num_triangles);


            // Get tile data
            glBindTexture(GL_TEXTURE_2D, highres_texture_id);
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

            // Write tile to result texture
            glBindTexture(GL_TEXTURE_2D, sim.result_id);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 4*x_offset, 4*y_offset, 4*sim.tile_w, 4*sim.tile_h, GL_RGB, GL_UNSIGNED_BYTE, data);
          }

          glBindFramebuffer(GL_FRAMEBUFFER, 0);
          glViewport(0, 0, settings.w, settings.h);
        }
        
        
        // If we've never been at this grid position before, we should start with some random paintings
        // If we have been here before, just copy the painting in storage
        if(sim.grid_paintings[sim.grid_pos].generation == 0)
        {
          for(unsigned int p = 0; p < sim.paintings.size(); ++p)
          {
            painting_randomise(&sim.paintings[p]);
          }
        }
        else
        {
          for(unsigned int p = 0; p < sim.paintings.size(); ++p)
          {
            painting_copy(&sim.paintings[p], &sim.grid_paintings[sim.grid_pos]);
          }
        }
        
        tile_x = sim.grid_pos%sim.grid_w;
        tile_y = sim.grid_pos/sim.grid_w;
        
        x_offset = tile_x * sim.tile_w;
        y_offset = tile_y * sim.tile_h;
        
        for(int y = 0; y < sim.tile_h; ++y)
        {
          for(int x = 0; x < sim.tile_w; ++x)
          {
            data[3*y*sim.tile_w + 3*x + 0] = sim.target.data[3*(y + y_offset) *sim.target.w + 3*(x + x_offset) + 0];
            data[3*y*sim.tile_w + 3*x + 1] = sim.target.data[3*(y + y_offset) *sim.target.w + 3*(x + x_offset) + 1];
            data[3*y*sim.tile_w + 3*x + 2] = sim.target.data[3*(y + y_offset) *sim.target.w + 3*(x + x_offset) + 2];
          }
        }
        
        glBindTexture(GL_TEXTURE_2D, texture_slice);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, sim.tile_w, sim.tile_h, GL_RGB, GL_UNSIGNED_BYTE, data);
        
        std::cout << std::endl;
        std::cout << "Pos: " << tile_x << " " << tile_y << std::endl;
        std::cout << "Score: " << sim.grid_paintings[sim.grid_pos].score << std::endl;
        std::cout << "Rate: " << sim.grid_paintings[sim.grid_pos].score_rate << std::endl;
        std::cout << "Gen: " << sim.grid_paintings[sim.grid_pos].generation << std::endl;
        std::cout << std::endl;

        last_grid_pos = sim.grid_pos;
      }
      
      
      // Draw paintings
      glViewport(0, 0, sim.tile_w, sim.tile_h);
      for(unsigned int p = 0; p < sim.paintings.size(); ++p)
      {
        glBindFramebuffer(GL_FRAMEBUFFER, sim.paintings[p].fbo);
        glClearColor(sim.paintings[p].r, sim.paintings[p].g, sim.paintings[p].b, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // bind the current vao
        glBindVertexArray(vao);
        
        // Vertices
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, 2 * 3 * sim.paintings[p].num_triangles * sizeof(GLfloat), &sim.paintings[p].positions[0], GL_STATIC_DRAW);
        
        // Colours
        glBindBuffer(GL_ARRAY_BUFFER, cbo);
        glBufferData(GL_ARRAY_BUFFER, 3 * 3 * sim.paintings[p].num_triangles * sizeof(GLfloat), &sim.paintings[p].colours[0], GL_STATIC_DRAW);
        
        // Texture coords
        glBindBuffer(GL_ARRAY_BUFFER, uvbo);
        glBufferData(GL_ARRAY_BUFFER, 2 * 3 * sim.paintings[p].num_triangles * sizeof(GLfloat), &sim.paintings[p].uvs[0], GL_STATIC_DRAW);
        
        glUseProgram(shader_program);
        
        // Set the uniforms
        glm::mat4 view = glm::ortho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(view));
        
        // Draw
        glDrawArrays(GL_TRIANGLES, 0, 3*sim.paintings[p].num_triangles);
      }
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glViewport(0, 0, settings.w, settings.h);


      // Comparisons
      for(unsigned int p = 0; p < sim.paintings.size(); ++p)
      {
        glUseProgram(similarity_program);

        glActiveTexture(GL_TEXTURE0 + texture_slice);
        glBindTexture(GL_TEXTURE_2D, texture_slice);

        glActiveTexture(GL_TEXTURE0 + sim.paintings[p].texture_id);
        glBindTexture(GL_TEXTURE_2D, sim.paintings[p].texture_id);

        // Setup uniforms
        glUniform1i(0, texture_slice);
        glUniform1i(1, sim.paintings[p].texture_id);
        glUniform1i(2, p);
    
        // Compute
        int groups_x = sim.tile_w/16;
        int groups_y = sim.tile_h/16;
    
        glDispatchCompute(groups_x, groups_y, 1);

        // Get scores
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, scores_bo);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, num_workgroups*sizeof(scores[0]), &scores[0]);

        // Calculate similarity percentage
        sim.paintings[p].score = 0.0;
        for(int n = 0; n < num_workgroups; ++n)
        {
          sim.paintings[p].score += scores[n];
        }
        sim.paintings[p].score = 1.0 - sim.paintings[p].score/(3*sim.tile_w*sim.tile_h);
      }
      glActiveTexture(GL_TEXTURE0);
      
      
      // Find best painting
      parent1_id = sim.paintings.size();
      parent1_score = FLT_MIN;
      parent2_id = sim.paintings.size();
      parent2_score = FLT_MIN;

      for(unsigned int p = 0; p < sim.paintings.size(); ++p)
      {
        if(sim.paintings[p].score >= parent1_score)
        {
          parent2_id = parent1_id;
          parent2_score = parent1_score;

          parent1_id = p;
          parent1_score = sim.paintings[p].score;
          continue;
        }
        if(sim.paintings[p].score >= parent2_score)
        {
          parent2_id = p;
          parent2_score = sim.paintings[p].score;
        }
      }

      assert(parent1_id != sim.paintings.size());
      assert(parent2_id != sim.paintings.size());
      assert(parent1_id != parent2_id);
      assert(parent1_score >= 0.0);
      assert(parent2_score >= 0.0);
      

      // To be used when a screenshot is taken
      settings.best_painting = sim.result_id;


      // Create new sim.paintings from best
      for(unsigned int p = 0; p < sim.paintings.size(); ++p)
      {
        if(p == parent1_id || p == parent2_id) {continue;}

        paintings_breed(&sim.paintings[p], &sim.paintings[parent1_id], &sim.paintings[parent2_id]);
      }
      
      
      // Mutate sim.paintings
      for(unsigned int p = 0; p < sim.paintings.size(); ++p)
      {
        if(p == parent1_id || p == parent2_id) {continue;}

        painting_jiggle(&sim.paintings[p]);
      }
      

      // Print scores occasionally
      if(sim.grid_paintings[sim.grid_pos].generation%250 == 0)
      {
        //std::cout << "Gen " << sim.grid_paintings[sim.grid_pos].generation << ": " << parent1_id << " - " << sim.grid_paintings[sim.grid_pos].score*100.0 << "% " << std::endl;
        //std::cout << "Gen " << sim.grid_paintings[sim.grid_pos].generation << ": " << parent1_id << " - " << sim.grid_paintings[sim.grid_pos].score*100.0 << "% " << sim.grid_paintings[sim.grid_pos].score_rate << std::endl;
      }


      // Save best painting if it's an improvement
      if(sim.paintings[parent1_id].score > sim.grid_paintings[sim.grid_pos].score)
      {
        int temp = sim.grid_paintings[sim.grid_pos].generation;
        painting_copy(&sim.grid_paintings[sim.grid_pos], &sim.paintings[parent1_id]);
        sim.grid_paintings[sim.grid_pos].generation = temp;
      }
      

      // Count generations
      sim.grid_paintings[sim.grid_pos].generation++;
      if(settings.mode == MODE_AUTO)
      {
        auto_generation++;
      }
    }


    // Render target and best sim.paintings
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glBindVertexArray(vao);
    
    glUseProgram(shader_program);
    
    glm::mat4 view = glm::ortho(-1.0, 1.0, 0.0, 1.0, -1.0, 1.0);
    glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(view));
    
    // Render target to the screen
    glBindTexture(GL_TEXTURE_2D, sim.target_id);
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
      unsigned int x_num = ceil(sqrt(sim.paintings.size()));
      unsigned int y_num = ceil(sqrt(sim.paintings.size()));

      if(x_num*(y_num-1) >= sim.paintings.size())
      {
        y_num -= 1;
      }

      float width  = 1.0/x_num;
      float height = 1.0/y_num;
      
      for(unsigned int y = 0; y < y_num; ++y)
      {
        for(unsigned int x = 0; x < x_num; ++x)
        {
          if(y*x_num + x >= sim.paintings.size()) {break;}
          
          glBindTexture(GL_TEXTURE_2D, sim.paintings[y*y_num + x].texture_id);
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
      // Draw the results texture
      glBindTexture(GL_TEXTURE_2D, sim.result_id);
      positions[0] = 0.0; positions[1] = 0.0;
      positions[2] = 0.0; positions[3] = 1.0;
      positions[4] = 1.0; positions[5] = 1.0;
      positions[6] = 1.0; positions[7] = 0.0;

      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), &positions, GL_STATIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, cbo);
      glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), &colours, GL_STATIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, uvbo);
      glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), &uvs, GL_STATIC_DRAW);
      glDrawArrays(GL_TRIANGLE_FAN, 0, 4);


      // Draw current best over the top of the results texture
      glBindTexture(GL_TEXTURE_2D, sim.paintings[parent1_id].texture_id);

      float x_gap = 1.0/sim.grid_w;
      float y_gap = 1.0/sim.grid_h;

      positions[0] = x_gap*tile_x;     positions[1] = y_gap*tile_y;
      positions[2] = x_gap*tile_x;     positions[3] = y_gap*(tile_y+1);
      positions[4] = x_gap*(tile_x+1); positions[5] = y_gap*(tile_y+1);
      positions[6] = x_gap*(tile_x+1); positions[7] = y_gap*tile_y;

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

  delete[] data;

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
