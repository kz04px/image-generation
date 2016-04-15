#include "defs.hpp"

void glfw_window_size_callback(GLFWwindow* window, int width, int height)
{
  //window_width = width;
  //window_height = height;

  glViewport(0, 0, width, height);
}

void glfw_cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
  //printf("%.4g %.4g\n", xpos, ypos);
}

void glfw_mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{ 
  // Positive is away, Negative is towards
}

void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
  // Button:
  // 0 - Left
  // 1 - Right
  // 2 - Middle

  // Action:
  // 1 - Down
  // 0 - Up
}

void glfw_keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  // Action:
  //  1 - Press
  //  2 - Repeat
  //  0 - Release

  switch(key)
  {
    case GLFW_KEY_ESCAPE:
      glfwSetWindowShouldClose(window, 1);
      break;
    case GLFW_KEY_F2:
      //screenshot_tga("test.tga", window_width, window_height);
      break;
    default:
      //printf("Key: %c(%i) %i\n", key, key, action);
      break;
  }
}
