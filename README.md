

### DESCRIBE

------

一个简单使用的，轻量级的、高性能的、具有可扩展性的简易日志库。

### Architecture

------

设计架构图如下图所示，采用多生产者单一消费者的异步模式，引入循环队列作为消息的缓冲区，提高吞吐量。

![arc](D:\My Documents\Desktop\cpp测试文件夹\杂七杂八typroa\arc.png)

### Usage

------

该logger库为header-only库，引入头文件即可，具体用法如下示例:

```cpp
#include "../src/logger.hpp"

/**
    @brief 通过宏定义的方式调用日志输出
    @param Msg 日志信息 支持左值和右值,推荐右值传参可提高性能
    LOGI -> yoyo::Logger::getInstance()->info(Msg);
    LOGE -> yoyo::Logger::getInstance()->error(Msg);
    LOGD -> yoyo::Logger::getInstance()->debug(Msg);
    LOGW -> yoyo::Logger::getInstance()->warn(Msg);
    LOGF -> yoyo::Logger::getInstance()->fatal(Msg);

    @brief 日志的配置选项
    yoyo::Logger::getInstance()->setWritefile(true).setConsle(true).setRotate(true).setFileNum(5);
    setWritefile -> 是否输出到文件
    setConsle -> 是否输出到控制台
    setRotate -> 是否开启日志文件的轮转
    setFileNum -> 日志文件的数量
    ....
*/

void output2File() {
  yoyo::Logger::getInstance()->setWritefile(false);
  std::string sMsg = "this is a test message";
  LOGI("This is a info log output to file");
  LOGE(sMsg);
  LOGD(std::move(sMsg));
  LOGT("This is a info log output to file");
  LOGW("This is a info log output to file");
  LOGF("This is a info log output to file");
}

void output2consle() {
  yoyo::Logger::getInstance()->setWritefile(false);
  yoyo::Logger::getInstance()->setConsle(true);
  std::string sMsg = "this is a test message";
  LOGI("This is a info log output to consle");
  LOGE(sMsg);
  LOGD(std::move(sMsg));
  LOGT("This is a info log output to consle");
  LOGW("This is a info log output to consle");
  LOGF("This is a info log output to consle");
}

int main() {
  output2File();
  output2consle();
  return 0;
}

```

###### 编译命令

```shell
g++ usage.cc -o a.out -std=c++20
```

### OUTPUT

------

输出格式为:

```c
[ timestamp ][ loglevel ][ fileName : function : line : logMsg ]
```

支持彩色终端文本输出，示例如下:

![image-20250117144815302](C:\Users\Administrator\AppData\Roaming\Typora\typora-user-images\image-20250117144815302.png)

支持输出文本到日志文件中，同时也可支持对文件进行分片操作，示例如下:

![image-20250117161750419](C:\Users\Administrator\AppData\Roaming\Typora\typora-user-images\image-20250117161750419.png)

### Performance

------

性能测试代码如下

```cpp
#include <random>
#include "./../src/logger.hpp"
using namespace yoyo;
const std::string sRandom =
    "qwertyuioplkjhgfdsazxcvbnm,[];.1234567890AQWERTYUIOPLKJHGFDSZXCVBNM";
constexpr size_t Msglenth = 64;
std::string getTestMessage() {
  std::random_device rd;
  std::mt19937 eng(rd());
  std::uniform_int_distribution<int> distr(0, sRandom.size() - 1);
  char Msg[Msglenth];
  for (int i = 0; i < Msglenth; i++) {
    Msg[i] = sRandom[distr(eng)];
  }
  return std::string(Msg);
}

constexpr size_t LogNum = 1000 * 1000;

void logTest(std::string Msg) {
  for (int i = 0; i < LogNum; i++) {
    LOGD(Msg);
    LOGE(Msg);
    LOGI(Msg);
    LOGT(Msg);
    LOGW(Msg);
  }
}

void calculPerformance(int threadum) {
  std::cout << "***************************************************************"
               "**********************"
            << std::endl;
  std::vector<std::thread> vThread;
  std::string msg = getTestMessage();
  auto start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < threadum; i++) {
    vThread.emplace_back(logTest, msg);
  }
  for (int i = 0; i < threadum; i++) {
    vThread[i].join();
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto consume =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  int num = ((LogNum * 5 * threadum * 1000) / consume.count());

  std::cout << "Process message: " << LogNum * 5 * threadum
            << " consume time: " << consume.count() << " ms" << std::endl;

  std::cout << "Total " << threadum << " threads "
            << "average: " << num << " per/second" << std::endl;

  std::cout << "Per thread average: " << num / 5 << " per/second" << std::endl;

  std::cout << "***************************************************************"
               "**********************"
            << std::endl;
}

int main() {
  calculPerformance(1);
  calculPerformance(4);
  calculPerformance(6);
  calculPerformance(8);
  calculPerformance(16);
}

```

单线程:

Process message: 5000000 consume time: 14708 ms
Total 1 threads average: 339951 per/second
Per thread average: 67990 per/second

*************************************************************************************

4线程:

Process message: 20000000 consume time: 61859 ms
Total 4 threads average: 323315 per/second
Per thread average: 64663 per/second

*************************************************************************************

6线程:

Process message: 30000000 consume time: 95497 ms
Total 6 threads average: 314145 per/second
Per thread average: 62829 per/second

*************************************************************************************

8线程:

Process message: 40000000 consume time: 115601 ms
Total 8 threads average: 346017 per/second
Per thread average: 69203 per/second

*************************************************************************************

16线程：

Process message: 80000000 consume time: 228147 ms
Total 16 threads average: 350651 per/second
Per thread average: 70130 per/second

本测试结果在i7-9700的虚拟机上得出。共写入文件26G大小，不同机器整体吞吐率差别较大。

### Todo

------

* 定向输入其他文件

* 将文件分片时，可采用覆盖回滚操作
* 进一步优化提高吞吐率