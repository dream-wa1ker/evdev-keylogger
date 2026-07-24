#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>


int main() {
    int fd = open("/dev/input/event3", O_RDONLY);
    if (fd < 0) { perror("open"); return 1; }

    struct input_event ev;
    while (read(fd, &ev, sizeof(ev)) == sizeof(ev)) {
        if (ev.type == EV_KEY && ev.value == 1) { // 1 = key down only
            fprintf(stdout, "[code:%d]", ev.code);
            fflush(stdout);
        }
    }
    close(fd);
    return 0;
}
