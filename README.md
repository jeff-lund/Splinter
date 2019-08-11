# Splinter

## To build

In order to build *Splinter*, just use type `make` and you will have a *client* and *server* made for you to use.

## To run,

In order to run *Splinter*, you have to start up the *server* first.

In order to this you will need to find your IPv4 Address under Wireless LAN adapter Wi-Fi

Example:

`./splinter start -a 127.0.0.1 -p 5731`

Once the server has started up, you can connect the client to it the same way

Exmaple:

`./splinter connect -a 127.0.0.1 -p 5731`
