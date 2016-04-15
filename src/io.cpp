#include "defs.hpp"

int load_bmp(s_texture* texture, const char *filename)
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