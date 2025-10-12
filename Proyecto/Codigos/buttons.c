#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <string.h>

#define DEVICE "/dev/input/event1"   // Select event

int main() {
    int fd;
    struct input_event ev;

    printf("Press button: PREV, PLAY, NEXT, VOL+, VOL-, MUTE, PAIR...\n");

    fd = open(DEVICE, O_RDONLY);
    if (fd < 0) {
        perror("No input device");
        return 1;
    }

    for (;;) {
        ssize_t n = read(fd, &ev, sizeof(struct input_event));
        if (n != sizeof(struct input_event))
            continue;

        if (ev.type == EV_KEY) {
            if (ev.value == 1) { // PRESIONADO
                switch (ev.code) {
                    case 412: 
                    	printf("PREV presionado\n"); 
                    	system("./doorbell.sh");
                    	system("./camera.sh");
                    	break;
                    case 207: 
                    	printf("PLAY presionado\n"); 
                    	system("pkill gst-launch-1.0");
                    	break;
                    case 407: printf("NEXT presionado\n"); break;
                    case 103: printf("VOL+ presionado\n"); break;
                    case 108: printf("VOL- presionado\n"); break;
                    case 113: printf("MUTE presionado\n"); break;
                    case 353: 
                    	printf("PAIR presionado\n"); 
                    	system("./led_off.sh");
                    	break;
                    case 352: 
                    	printf("ACT presionado\n"); 
                    	system("./led_on.sh");
                    	break;
                    default: printf("Unknown button (code=%d)\n", ev.code); break;
                }
            } 
        }
    }

    close(fd);
    return 0;
}

