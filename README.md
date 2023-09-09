# QT-tcp_ServerAndClient

局域网多客户端与服务器间收发文件或信息，客户端使用了Qt的两种多线程, 服务器使用了Qt提供的线程池

环境：qt5.15.2,win11,cmake(需要修改cmakelist中qt安装路径）

端口设为了8000（消息）和8001（文件）
文件接收默认地址为当前应用程序所在路径，用的是QDir::currentPath();
消息文件发送机制看 command.h 

对不起,我是狗,有一堆bug没有修
