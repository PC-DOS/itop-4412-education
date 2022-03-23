# 东南大学生物科学与医学工程学院ARM实验附件

本实验基于iTop-4412开发板。

编译器：arm-2009q3

## 拉取/更新代码

实验用代码可能视具体情况进行更新，推荐在虚拟机内直接拉取代码。具体流程如下：

1. 登入虚拟机，启动一个终端。
   
2. 依次执行下面的命令：

```
cd ~
mkdir iTop-4412-Expr
cd iTop-4412-Expr
git clone http://share.bmeonline.cn/213170761/itop-4412-education.git ./
```

3. 如果您先前已经执行过上面的命令，但是需要获取代码更新，请在终端中依次执行下面的命令：

```
cd ~/iTop-4412-Expr
git fetch origin master
git pull origin master
```

至此，您应该已经在当前用户的主文件夹下，获取了一个名为“`iTop-4412-Expr`”的目录，并且其中包含实验所需的数据。
从这里开始，假设实验所需的资料、源代码等全部位于当前用户的主文件夹下的“`iTop-4412-Expr`”的目录，即路径“`~/iTop-4412-Expr`”。
