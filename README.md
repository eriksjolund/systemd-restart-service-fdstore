# systemd-restart-service-fdstore

Example systemd user service that uses FDSTORE=1


# Installation

```
git clone URL
cd systemd-restart-service-fdstore
gcc -o /tmp/server server_fdstore.c -l systemd
mkdir -p ~/.config/systemd/user
cp upcase@.service ~/.config/systemd/user
cp upcase@.socket  ~/.config/systemd/user
systemctl --user daemon-reload
Systemctl --user start upcase@demo.socket
```

# Usage


```
$ socat  readline tcp4:127.0.0.1:3010
aaaaa
AAAAA
bbb
BBB
ccc
CCC
dddd
DDDD
eeeeeee
EEEEEEE
fffffff
FFFFFFF
```


The systemd user service  upcase@demo.service calls `exit(EXIT_FAILURE)` every nth time (n=3) it receives a string.
systemd will then pass the previously stored accepted TCP socket when restarting the service.
The client will not notice the restart because the TCP connections stays active.


In another shell run

```
journalctl  --user -xe -u upcase@demo.service
```
to see the restarts happening.
