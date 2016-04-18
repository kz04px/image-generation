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

  switch(key)
  {
    case GLFW_KEY_ESCAPE:
      glfwSetWindowShouldClose(window, 1);
      break;
    case GLFW_KEY_F2:
      //bmp_save_n(1, "test1.bmp");
      bmp_save_n(settings->best_painting, "result.bmp");
      break;
    case GLFW_KEY_SPACE:
      settings->paused = !settings->paused;
      break;
    case GLFW_KEY_ENTER:
      settings->tiled_view = !settings->tiled_view;
      break;
    default:
      break;
  }
}
