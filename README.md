# QT-tcp_ServerAndClient

局域网多客户端与服务器间收发文件或信息，使用了多线程，目前多文件发送未完成
环境：qt5.15.2,cmake,win11

端口设为了8000（消息）和8001（文件）
文件接收默认地址为当前应用程序所在路径，用的是QDir::currentPath();
