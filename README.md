# qr-wireguard-ui
<span style="color:#d3d3d3">Quantum Resistant WireGuard UI for NanoPi</span>
```
wireguard-ui(open source project) + webUI backend + quantum resistant wireguard kernel on NanoPi
It's currently in development. 😎

```

## How to build
```
$ vi ./build_qrwg.sh
YOUR_LOCAL_PATH=XXX   <------- Fix this toolchain path with yours.
TOOLCHAIN_PATH=$YOUR_LOCAL_PATH/friendlywrt23-rk3328/friendlywrt/staging_dir/toolchain-aarch64_generic_gcc-12.3.0_musl/bin
...
~

$ ./build_qrwg.sh release
...

$ cd output
$ ls -l
-rw-rw-r-- 1 chyi chyi 14936118  1월  2 16:15 qr_wireguard_0.9.00.tar.gz

```

## How to install on NanoPi
```
<Ubuntu 22.04 LTS>
$ scp ./qr_wireguard_0.9.00.tar.gz root@192.168.2.1:~/workspace
$ ssh root@192.168.2.1
<NanoPi R2S Plus>
# cd /root/workspace
# tar xvzf qr_wireguard_0.9.00.tar.gz
# cd qr_install
# ./Install.sh

Good luck~
Slowboot
```

## My blog postings for this project
  For more information, please read the blog posting below.<br>
  https://slowbootkernelhacks.blogspot.com/2025/01/quantum-resistant-wireguard-from-ui-to.html
  <br>

## Reference codes
```
  a) webui project by ngoduykhanh Khanh Ngo
     https://github.com/ngoduykhanh/wireguard-ui
  b) my quantum resistant layer 2 wireguard project
     https://github.com/ChunghanYi/qr-l2-wireguard
```
  <br>

  __WireGuard is a registered trademark of Jason A. Donenfeld.__

