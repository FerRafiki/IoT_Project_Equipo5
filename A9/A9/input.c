#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <linux/input.h>
#include <sys/time.h>


#define LEN 80
#define ELEMENTS 4

void fread_file() {
    FILE *fin = fopen("/dev/input/event0", "rb");
    if (!fin) {
        perror("Error al abrir archivo para lectura");
        exit(EXIT_FAILURE);
    }

    struct input_event buffer[ELEMENTS];
    size_t readCount = fread(buffer, sizeof(struct input_event), ELEMENTS, fin);
    fclose(fin);

    // Despliegue solo de los elementos realmente leídos
    for (size_t i = 0; i < readCount; i++) {
        printf("time_sec=%d, time_usec=%d, type=%d, code=%d, value)%d\n",
               buffer[i].time.tv_sec, buffer[i].time.tv_usec, buffer[i].type, buffer[i].code, buffer[i].value);
    }
}

int main(int argc, char * argv[]){
        fread_file();
}
