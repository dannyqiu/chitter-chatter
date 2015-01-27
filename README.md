# chitter-chatter

An upgrade to instant messaging where channels are in style. Create a channel, have your friends join it, and then talk away!  
*A Systems project by Stanley Lok, Danny Qiu, and Ivy Wong.*

### Requirements

- [GTK+](http://www.gtk.org)
    - Install on Ubuntu using `sudo apt-get install build-essential gnome-devel`

### Running

`make` to build both the client and the server

`make client` to build the client and `/build/client` to run the client

`make server` to build the server and `/build/server` to run the server

### BUGS

- While working with GUI's we encountered the inherent "Heisenbugs" where the program would SEGFAULT and we would be unable to reproduce the error. Please use this at your own risk.

### Notes

- Since this is a simple chat server/client model, we don't have cross-compability support. Therefore, optimal performance is reached when all machines are the same. This will be improved in a future version when we implement packing for chat packets
- We would like to add file sharing and other cool features if we had more time to work on this project.
