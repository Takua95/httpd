# httpd
Hypertext Transfer Protocol Daemon

# building
Run make, which creates the build directory, which will contain the server binary.

# running server
Server assumes that HTML content is in ../demo_content (relative to the server binary location).

It's possible to run:
```
make && build/server
```
to make and run the server in one go. 