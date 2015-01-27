# chitter-chatter

An upgrade to instant messaging where channels are in style. Create a channel, have your friends join it, and then talk away!  
*A Systems project by Stanley Lok, Danny Qiu, and Ivy Wong in Period 8 Systems-Level Programming.*

### Requirements

- [GTK+](http://www.gtk.org)
    - Install on Ubuntu using `sudo apt-get install build-essential gnome-devel`
    - Other platforms need to have GTK+ compiled indepently. See installation details [http://www.gtk.org/download/index.php](here)

### Running

`make` to build both the client and the server

`make client` to build the client and `/build/client` to run the client

`make server` to build the server and `/build/server` to run the server

### BUGS

- While working with GUI's we encountered the inherent "Heisenbugs" where the program would SEGFAULT and we would be unable to reproduce the error. Please use this at your own risk.
    - We mostly see the SEGFAULTS occuring in creating and joining channels due to the way GTK+'s `TreeView` works.

### List of Files

- `constants.h` has constants and anything used in both the client/server
    - If using multiple computers, `SERVER_IP` must match the IP Address of the computer running the server
- `server.h, server.c` contains the code for running the server
- `gclient.h, gclient.c`has functions for the client to connect to the server as well as sending data
    - It was originally going be to text-based, but that code is commented out in favor of a GUI
- `gui.h, gui.c` are the GUI interface wrappers for function in gclient (And honestly more difficult to understand than we ever expected)
- `layout.ui` is the builder file used for constructing the GUI
- `util.h, util.c` contain useful functions that will probably be used often

### Notes

- Since this is a simple chat server/client model, we don't have cross-compability support. Therefore, optimal performance is reached when all machines are the same. This will be improved in a future version when we implement packing for chat packets
- We would like to add file sharing and other cool features if we had more time to work on this project.
