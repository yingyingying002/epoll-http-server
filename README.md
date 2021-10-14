# epoll-http-server
linux环境下epoll实现的接收http请求的服务器，有简单客户端用于测试。

客户端通过curl命令发送http请求，服务端epoll接受请求，调用tinyHttp库(简单的http开源项目)解析http请求并返回response。
练手用，所以简单试了下多进程和多线程的语法，感觉效率不怎么样。

编译使用cmake，tinyhttp生成静态库，cmake编译生成client.out和servet.out两个可执行文件。

编译命令:

mkdir build && cd build

cmake .. .

make
