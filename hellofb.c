#include <linux/fb.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

int fb_fd;
struct fb_fix_screeninfo finfo;
struct fb_var_screeninfo vinfo;
long screensize;
uint8_t *fbp;

static void init(void) {
  fb_fd = open("/dev/fb0",O_RDWR);
  //Get variable screen information
  ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
  vinfo.grayscale=0;
  vinfo.bits_per_pixel=32;
  ioctl(fb_fd, FBIOPUT_VSCREENINFO, &vinfo);
  ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
  ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo);
  screensize = vinfo.yres_virtual * finfo.line_length;
  fbp = mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, (off_t)0);
}

static inline int putpixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
  if (x >= vinfo.xres || x < 0 || y >= vinfo.yres || y < 0) return -1;
  long location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) + (y+vinfo.yoffset) * finfo.line_length;
  uint32_t color = (r<<vinfo.red.offset) | (g<<vinfo.green.offset) | (b<<vinfo.blue.offset);
  *((uint32_t*)(fbp + location)) = color;
  return 0;
}

#define FORXY for (x=0;x<vinfo.xres;x++) for (y=0;y<vinfo.yres;y++)
#define FORXY_SQUARE for (x=0;x<size;x++) for (y=0;y<size;y++)

void draw_circle(int size) {
  int c = size/2;
  // (y-c)^2+(x-c)^2 = c^2
  // (y-c)^2 = c^2-(x-c)^2
  // y = c +- sqrt(c^2-(x-c)^2)
  int c2 = c*c, x, y;
  for (x = 0; x < size; x++) {
    int d = (int)sqrt(c2-(x-c)*(x-c));
    putpixel(x, c+d, 0xFF, 0x00, 0x00);
    putpixel(x, c-d, 0xFF, 0x00, 0x00);
  }
}

void draw_line(int x0, int y0, int x1, int y1) {
  int k = (y1-y0)/(x1-x0), i, x, y;
  for (i = 1; ; i++) {
    x = x0 + i;
    y = y0 + k*i;
    if (x < 0 || x > x1) break;
    putpixel(x, y, 0xFF, 0x00, 0x00);
  }
}
  
int main(void) {
  init();
    
  int x,y;

  FORXY putpixel(x, y, 0x00, 0x00, 0x00);
  
  int size = vinfo.xres > vinfo.yres ? vinfo.yres : vinfo.xres;
  
  // a circle
  int i;
  for (i = -2; i <= 2; ++i) draw_circle(size+i);

  // a lambda
  draw_line(0, size, size/2, size/2);
  draw_line(0, 0, size, size);
  
  return 0;
}
