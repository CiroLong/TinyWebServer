# TinyWebServer

TinyWebServer 是一个基于 Linux 的高性能 C++ Web 服务器，支持多线程、异步日志、MySQL 连接池、定时器等功能，适合学习和实践高性能服务器编程。

## 特性
- 支持 HTTP/1.1 协议，静态资源访问
- 基于 epoll 的高效 I/O 复用
- 线程池处理并发请求
- 支持 MySQL 用户认证，连接池管理
- 支持异步/同步日志系统
- 支持连接超时管理（定时器）
- 支持优雅关闭
- 代码结构清晰，易于扩展

## 目录结构

- **code/**  核心源代码
    - **buffer/**：高效缓冲区管理，实现网络数据的高效读写。
    - **config/**：配置相关（预留，当前为空）。
    - **http/**：HTTP 协议处理，包括请求解析（HttpRequest）、响应生成（HttpResponse）、连接管理（HttpConn）。
    - **log/**：日志系统，支持异步/同步日志写入，便于调试和运行监控。
    - **pool/**：资源池，包括线程池（ThreadPool）和 MySQL 数据库连接池（SqlConnPool、SqlConnRAII）。
    - **server/**：服务器主控模块，包括 epoll 封装（Epoller）和 WebServer 主体逻辑。
    - **timer/**：定时器模块，基于小根堆实现，用于管理连接超时。
    - **main.cpp**：程序入口，启动 WebServer。
- **log/**        日志输出目录
- **build/**      构建相关文件
- **test/**       单元测试与功能测试
- **bin/**        可执行文件输出目录

## 快速开始

### 依赖
- Linux 环境
- g++ (建议 7.0 及以上)
- GNU Make
- MySQL 开发库（libmysqlclient）

### 编译
```bash
make
```
编译完成后，`bin/server` 为主程序。

### 运行
1. 确保 MySQL 服务已启动，并创建数据库及 user 表：
```sql
CREATE DATABASE webserver;
USE webserver;
CREATE TABLE user(
    username VARCHAR(50) PRIMARY KEY,
    password VARCHAR(50) NOT NULL
);
```
2. 启动服务器：
```bash
./bin/server
```
3. 浏览器访问 `http://<服务器IP>:1316/`，即可看到首页。

### 配置说明
- 端口、线程数、数据库参数等可在 `main.cpp` 中修改。
- 静态资源目录为 `resources/`，可自行添加 html/css/js 文件。

## 使用 CMake 编译

1. 安装依赖（如未安装）：
   - g++
   - cmake
   - MySQL 开发库（libmysqlclient-dev）

2. 编译步骤：
```bash
mkdir -p build
cd build
cmake ..
make
```
编译完成后，`../bin/server` 为主程序。

## 主要模块说明
- **WebServer**：服务器主控类，负责 socket 初始化、事件循环、连接管理等。
- **ThreadPool**：线程池，负责任务调度与并发处理。
- **Epoller**：epoll 封装，实现高效 I/O 事件监听。
- **HttpRequest/HttpResponse**：HTTP 请求解析与响应生成。
- **SqlConnPool**：MySQL 连接池，支持多线程安全的数据库访问。
- **HeapTimer**：定时器，管理连接超时。
- **Log**：日志系统，支持异步写入。

## 参考与致谢
- 本项目参考了网络上多种高性能服务器设计思想，用于学习操作系统、网络编程、后端开发等相关知识。

## License
MIT
