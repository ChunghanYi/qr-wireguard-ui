## backend C++ agent for wireguard-ui
```
This web-agentd(C++11 version) is used to communicate with wireguard-ui.
```

## How to build for NanoPi(arm64)
```
<Ubuntu 22.04 LTS>
$ ./build_arm64.sh
$ cd build
$ ls -l web-agentd

$ ./web-agentd -h
Usage: web-agentd [OPTION]
Options
 -f, --foreground    in foreground
 -d, --daemon        fork in background
 -v, --version       show version information and exit

```

## Reference codes
This project is based on the following projects

```
<Simple-TCP-Server-Client-CPP-Example>
https://github.com/elhayra/tcp_server_client

<Fast C++ logging library>
https://github.com/gabime/spdlog
```
