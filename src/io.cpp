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
  int bpp = 24;
  int row_size = int((bpp*w+31)/32)*4;
  int padding = row_size - 3*w;

  assert(0 <= padding && padding <= 3);

  if(image_size == 0) {image_size = 3*w*h;}
  if(data_pos == 0) {data_pos = 54;}
  //fseek(file, data_pos);

  unsigned char* data = new unsigned char[3 * w * h * 2];
  if(data == NULL) {fclose(file); return -5;}
  
  unsigned char junk[3];

  int total = 0;

  //int r = fread(data, 1, 3 * w * h, file);
  for(int y = 0; y < h; ++y)
  {
    total += fread(data + y*3*w, sizeof(*data), 3*w, file);
    if(padding != 0)
    {
      fread(junk, sizeof(*junk), padding, file);
    }
  }

  assert((3*w+padding)*h == image_size);
  assert(ftell(file) == 54 + image_size);

  // Swap from BGR to RGB
  for(int i = 0; i < 3 * w * h; i += 3)
  {
    // 0.21R + 0.72G + 0.07B
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

  FILE *file = fopen(filename, "wb");
  if(file == NULL) {return -1;}

  glActiveTexture(GL_TEXTURE0 + n);
  glBindTexture(GL_TEXTURE_2D, n);

  GLint width = -1;
  GLint height = -1;

  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

  assert(width != -1);
  assert(height != -1);

  unsigned char header[54] = {0};
  GLubyte *data = new GLubyte[3*width*height*2]();
  if(data == NULL) {return -1;}

  int bpp = 24;
  int row_size = int((bpp*width+31)/32)*4;
  int padding = row_size - 3*width;

  assert(0 <= padding && padding <= 3);

  int bitmap_size = (3*width+padding)*height*sizeof(*data);
  int size = 54*sizeof(*header) + bitmap_size; // Total size of file in bytes
  GLubyte empty[3] = {0x01, 0x01, 0x01};

  // Header data
  header[0x0] = 'B';
  header[0x1] = 'M';
  header[0x2] = size;
  header[0x3] = size>>8;
  header[0x4] = size>>16;
  header[0x5] = size>>24;
  header[0xA] = 54;          // Offset to image data
  header[0xE] = 40;          // Size of DIB header
  header[0x12] = width;      // Width
  header[0x13] = width>>8;   // Width
  header[0x14] = width>>16;  // Width
  header[0x15] = width>>24;  // Width

  header[0x16] = height;     // Height
  header[0x17] = height>>8;  // Height
  header[0x18] = height>>16; // Height
  header[0x19] = height>>24; // Height

  header[0x1A] = 1;          // Number of colour planes
  header[0x1C] = bpp;        // Bits Per Pixel (bpp)
  header[0x1E] = 0;          // 
  header[0x22] = bitmap_size;     // 
  header[0x23] = bitmap_size>>8;  // 
  header[0x24] = bitmap_size>>16; // 
  header[0x25] = bitmap_size>>24; // 
  header[0x26] = 0xC4;
  header[0x27] = 0x0E;
  header[0x2A] = 0xC4;
  header[0x2B] = 0x0E;

  // Pixel data
  glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

  fwrite(header, sizeof(*header), 54, file);
  
  for(int y = 0; y < height; ++y)
  {
    fwrite(data + y*3*width, sizeof(*data), 3*width, file);

    if(padding > 0)
    {
      fwrite(empty, sizeof(*empty), padding, file);
    }
  }

  assert(ftell(file) == 54 + (3*width+padding)*height);

  delete[] data;
  fclose(file);
  
  return 0;
}
