C++神武辅助程序
Windows MSVC 2015 x86
鼠标键盘模拟容易被游戏检测，为此配套自己写的Stm32HID设备以控制鼠标键盘，实现辅助。
不支持后台，由于游戏有钩子检测，抓取图片时取整个屏幕的图像。频繁访问的资源文件在程序运行时加载到内存减少IO读写，HID通信加锁，同时跑2个客户端。
Opencv识别图片，
OCR识别数字
任务栏坐标OCR识别正确率低，采用图像识别坐标
能跑的日常：
师门任务、狐狸、修炼任务、宝图任务、捉鬼任务(没有自动组人功能)

编译平台：debug x86
使用到的库：leptonica、Tesseract-OCR、opencv2
