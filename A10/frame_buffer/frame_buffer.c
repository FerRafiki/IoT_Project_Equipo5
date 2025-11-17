#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <stdint.h>

//Los argumetos son de 0 a 255, se manda r g y b
int main(int argc, char *argv[]) {
    int fd = open("/dev/fb0", O_RDWR);
    if (fd < 0) { perror("open"); return 1; }

    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    ioctl(fd, FBIOGET_FSCREENINFO, &finfo);
    ioctl(fd, FBIOGET_VSCREENINFO, &vinfo);

    long screensize = finfo.line_length * vinfo.yres;
    uint32_t *fbp = mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if ((intptr_t)fbp == -1) { perror("mmap"); return 1; }

    // Sin color por default
    uint8_t r = 0, g = 0, b = 0;
    //Recibir los valores dados a cada color
    if (argc == 4) { // Permite ./a.out R G B
        r = atoi(argv[1]);
        g = atoi(argv[2]);
        b = atoi(argv[3]);
    }
	//Asignar combinacion de colores
    uint32_t color = (r << vinfo.red.offset) |
                     (g << vinfo.green.offset) |
                     (b << vinfo.blue.offset);

    long pixels = (screensize / (vinfo.bits_per_pixel / 8));
    for (long i = 0; i < pixels; i++)
        fbp[i] = color;

    munmap(fbp, screensize);
    close(fd);
    return 0;
}
