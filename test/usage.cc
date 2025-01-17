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