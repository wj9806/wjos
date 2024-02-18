## ubuntu 环境搭建
安装GCC编译器和CMake：
```shell
sudo apt-get install gcc-i686-linux-gnu -y
sudo apt-get install gdb -y
sudo apt-get install cmake -y
```
安装qemu：
```shell
sudo apt-get install qemu-system-x86 -y
```
安装vscode：https://code.visualstudio.com，点击下载deb包
```shell
#执行安装命令
sudo dpkg -i code_安装包名.deb
```
配置sudo:
```shell
sudo vi /etc/sudoers

#文件中添加一行,username为你当前系统登陆用户名
username ALL=(ALL) NOPASSWD: /usr/bin/mount, /usr/bin/umount, /usr/bin/cp
```
