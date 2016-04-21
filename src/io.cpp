#include "defs.hpp"

std::string file_load(const char* filename)
{
  std::ifstream t(filename);
  std::stringstream buffer;
  buffer << t.rdbuf();
  return buffer.str();
}

bool vertex_shader_load(GLuint* vertex_shader, const char* filename)
{
  assert(vertex_shader != NULL);
  assert(filename != NULL);

  std::string vertex_source = file_load(filename);

  *vertex_shader = glCreateShader(GL_VERTEX_SHADER);

  const char *source = vertex_source.c_str();
  int length = vertex_source.size();

  glShaderSource(*vertex_shader, 1, &source, &length);
  glCompileShader(*vertex_shader);
  if(!check_shader_compile_status(*vertex_shader))
  {
    return false;
  }

  return true;
}

bool fragment_shader_load(GLuint* fragment_shader, const char* filename)
{
  assert(fragment_shader != NULL);
  assert(filename != NULL);
  
  std::string fragment_source = file_load(filename);

  *fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

  const char *source = fragment_source.c_str();
  int length = fragment_source.size();

  glShaderSource(*fragment_shader, 1, &source, &length);
  glCompileShader(*fragment_shader);
  if(!check_shader_compile_status(*fragment_shader))
  {
    return false;
  }

  return true;
}

bool compute_shader_load(GLuint* compute_shader, const char* filename)
{
  assert(compute_shader != NULL);
  assert(filename != NULL);
  
  std::string compute_source = file_load(filename);

  *compute_shader = glCreateShader(GL_COMPUTE_SHADER);

  const char *source = compute_source.c_str();
  int length = compute_source.size();

  glShaderSource(*compute_shader, 1, &source, &length);
  glCompileShader(*compute_shader);
  if(!check_shader_compile_status(*compute_shader))
  {
    return false;
  }

  return true;
}

int bmp_load(s_texture* texture, const char *filename)
{
  if(filename == NULL) {return -1;}
  FILE *file = fopen(filename, "rb");
  if(file == NULL) {return -2;}

  unsigned char header[54];
  if(fread(header, 1, 54, file) != 54) {fclose(file); return -3;}
  if(header[0] != 'B' || header[1] != 'M') {fclose(file); return -4;}

  int data_pos   = *(int*)&(header[0x0A]);
  int image_size = *(int*)&(header[0x22]);
  int w      = *(int*)&(header[0x12]);
  int h     = *(int*)&(header[0x16]);
  //int bpp        = *(int*)&(header[0x1C]); // bits per pixel

  if(image_size == 0) {image_size = 3*w*h;}
  if(data_pos == 0) {data_pos = 54;}

  unsigned char* data = new unsigned char[image_size * sizeof(*data)];
  if(data == NULL) {fclose(file); return -5;}
  int r = fread(data, 1, image_size, file);
  if(r != image_size) {fclose(file); return -6;}
  
  // Swap from BGR to RGB
  for(int i = 0; i < image_size; i += 3)
  {
    // 0.21 R + 0.72 G + 0.07 B.
    #ifdef GRAYSCALE
    float gray = 0.07*data[i+0] + 0.72*data[i+1] + 0.21*data[i+2];
    data[i+0] = gray;
    data[i+1] = gray;
    data[i+2] = gray;
    #endif

    unsigned char temp = data[i+0];
    data[i+0] = data[i+2];
    data[i+2] = temp;
  }
  
  fclose(file);

  texture->w = w;
  texture->h = h;
  texture->data = data;

  return 0;
}

int bmp_save_n(int n, const char *filename)
{
  assert(filename != NULL);
  assert(n >= 0);

  glActiveTexture(GL_TEXTURE0 + n);
  glBindTexture(GL_TEXTURE_2D, n);

  GLint width = -1;
  GLint height = -1;

  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

  assert(width != -1);
  assert(height != -1);

  GLubyte data[width*height*3];
  unsigned char header[54] = {0};

  int size = 54*sizeof(*header) + 3*width*height*sizeof(*data); // Total size of file in bytes
  int bpp = 24;

  // Header data
  header[0] = 'B';
  header[1] = 'M';
  header[2] = size;
  header[3] = size>>8;
  header[4] = size>>16;
  header[5] = size>>24;
  header[10] = 54;         // Offset to image data
  header[14] = 12;         // Size of DIB header
  header[18] = width;      // Width
  header[19] = width>>8;   // Width
  header[20] = height;     // Height
  header[21] = height>>8;  // Height
  header[22] = 1;          // Number of colour planes
  header[24] = bpp;        // Bits Per Pixel (bpp)

  // Pixel data
  glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

  FILE *file = fopen(filename, "w");
  if(file == NULL) {return -1;}

  fwrite(header, sizeof(*header), 54, file);
  //fwrite(data, sizeof(*data), width*height*3, file);

  //int row_size = int((bpp*width+31)/32)*4;
  //int padding[row_size - 3*width] = {0};

  for(int y = 0; y < height; ++y)
  {
    fwrite(data + y*(3*width), sizeof(*data), 3*width, file);
    //fwrite(padding, sizeof(*data), row_size - 3*width, file);
  }

  fclose(file);
  
  return 0;
}