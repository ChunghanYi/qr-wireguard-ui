# qr-wireguard-ui
<span style="color:#d3d3d3">Quantum Resistant WireGuard UI</span>
```
wireguard-ui(open source project) + webUI backend + quantum resistant wireguard kernel
It's currently in development.

```

## How to build
  Caution: <br>
  You must edit the build_qrwg.sh file for your cross-toolchain path.<br><br>

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
$ scp ./qr_wireguard_0.9.00.tar.gz root@192.168.2.1:~/workspace
$ ssh root@192.168.2.1
$ cd /root/workspace
$ tar xvzf qr_wireguard_0.9.00.tar.gz
$ cd qr_install
$ ./Install.sh

```

## My blog postings for this project
  For more information, please read the blog posting below.<br>
  <br>

## Reference codes
  https://github.com/ngoduykhanh/wireguard-ui <br>
  <br>

  __WireGuard is a registered trademark of Jason A. Donenfeld.__

