#include "defs.hpp"

void glfw_window_size_callback(GLFWwindow* window, int width, int height)
{
  s_settings *settings = (s_settings*)glfwGetWindowUserPointer(window);
  if(settings == NULL) {return;}

  settings->w = width;
  settings->h = height;

  glViewport(0, 0, width, height);
}

void glfw_cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
}

void glfw_mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{ 
}

void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
}

void glfw_keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  // Action:
  //  1 - Press
  //  2 - Repeat
  //  0 - Release

  if(action != 1)
  {
    return;
  }
  
  s_settings *settings = (s_settings*)glfwGetWindowUserPointer(window);
  if(settings == NULL) {return;}

  float average = 0.0;
  float lowest = FLT_MAX;
  float highest = FLT_MIN;
  int count = 0;

  switch(key)
  {
    case GLFW_KEY_ESCAPE:
      glfwSetWindowShouldClose(window, 1);
      break;
    case GLFW_KEY_F2:
      //bmp_save_n(1, "test1.bmp");
      //bmp_save_n(settings->best_painting, "result.bmp");
      bmp_save_n(settings->sim->result_id, "result.bmp");
      break;
    case GLFW_KEY_F5:
      std::cout << "Saving" << std::endl;
      paintings_save(settings->sim, "save.dat");
      break;
    case GLFW_KEY_F9:
      std::cout << "Loading" << std::endl;
      paintings_load(settings->sim, "save.dat");
      break;
    case GLFW_KEY_SPACE:
      settings->paused = !settings->paused;
      break;
    case GLFW_KEY_ENTER:
      settings->tiled_view = !settings->tiled_view;
      break;
    case GLFW_KEY_1:
      std::cout << "Mode: Manual" << std::endl;
      std::cout << std::endl;
      settings->mode = MODE_MANUAL;
      break;
    case GLFW_KEY_2:
      std::cout << "Mode: Auto" << std::endl;
      std::cout << std::endl;
      settings->mode = MODE_AUTO;
      break;
    case GLFW_KEY_T:
      for(unsigned int p = 0; p < settings->sim->grid_paintings.size(); ++p)
      {
        if(settings->sim->grid_paintings[p].generation == 0) {continue;}

        average += settings->sim->grid_paintings[p].score;
        count++;

        if(settings->sim->grid_paintings[p].score < lowest)  {lowest = settings->sim->grid_paintings[p].score;}
        if(settings->sim->grid_paintings[p].score > highest) {highest = settings->sim->grid_paintings[p].score;}
      }

      if(count > 0)
      {
        average = average/count;
      }
      else
      {
        average = 0.0;
      }

      std::cout << "Count: " << count << std::endl;
      std::cout << "Avg:   " << average << std::endl;
      std::cout << "Low:   " << lowest << std::endl;
      std::cout << "High:  " << highest << std::endl;
      std::cout << std::endl;
      break;
    case GLFW_KEY_R:
      painting_randomise(&settings->sim->grid_paintings[settings->sim->grid_pos]);
      for(unsigned int p = 0; p < settings->sim->paintings.size(); ++p)
      {
        painting_randomise(&settings->sim->paintings[p]);
      }
      break;
    case GLFW_KEY_W:
    case GLFW_KEY_UP:
      settings->mode = MODE_MANUAL;
      if(settings->sim->grid_pos + settings->sim->grid_w < settings->sim->grid_paintings.size())
      {
        settings->sim->grid_pos += settings->sim->grid_w;
      }
      break;
    case GLFW_KEY_A:
    case GLFW_KEY_LEFT:
      settings->mode = MODE_MANUAL;
      if((settings->sim->grid_pos)%(settings->sim->grid_w) != 0)
      {
        settings->sim->grid_pos--;
      }
      break;
    case GLFW_KEY_S:
    case GLFW_KEY_DOWN:
      settings->mode = MODE_MANUAL;
      if(settings->sim->grid_pos - settings->sim->grid_w >= 0)
      {
        settings->sim->grid_pos -= settings->sim->grid_w;
      }
      break;
    case GLFW_KEY_D:
    case GLFW_KEY_RIGHT:
      settings->mode = MODE_MANUAL;
      if((settings->sim->grid_pos+1)%(settings->sim->grid_w) != 0)
      {
        settings->sim->grid_pos++;
      }
      break;
    default:
      break;
  }
}
