# 第一个Linux程序：Hello World

启动虚拟机，从Ubuntu桌面左侧的快速启动栏打开“终端”，并执行下面的命令：

```
cd ~/iTop-4412-Expr/Expr02-HelloWorld/
make
```

此时，应该能在“`~/iTop-4412-Expr/Expr02-HelloWorld/`”中看到编译生成的可执行文件“`helloworld`”，在终端中执行“`file helloworld`”指令，可以看到该可执行文件具有“`ELF 32-bit LSB executable, ARM`”架构。

接下来，请将一个已格式化为FAT32格式的U盘或SD卡插入计算机，VMware询问是否要挂载到虚拟机时请选择将其挂载到主机。随后，请从虚拟机中将编译生成的可执行文件“`helloworld`”复制到插入的磁盘，您通常可以简单地使用在虚拟机中复制、在主机上粘贴的方法来完成。如果存在问题，请考虑使用VMware提供的共享文件夹功能。

将磁盘插入开发板，在超级终端中执行：

```
cd /mnt/sda1
ls
./helloworld
```

您应该能看到超级终端上列出了USB磁盘内的文件，并且打印了“`Hello World!`”文本。

在超级终端中执行“`cd /`”命令并移除插入的磁盘，实验完毕。

本实验的进一步资料可以参考随附的迅为手册：iTOP-4412精英版光盘资料\itop-4412开发板之精英版使用手册_v4.0.pdf的第十章“嵌入式 Linux 系统编程（应用开发）”的第10.3节“第一个程序‘helloworld’”。
