
#include "logger.hpp"
#include <random>
using namespace yoyo;
const std::string sRandom = "qwertyuioplkjhgfdsazxcvbnm,[];.1234567890AQWERTYUIOPLKJHGFDSZXCVBNM";

constexpr size_t Msglenth = 64;

std::string getTestMessage(){
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<int> distr(0, sRandom.size() - 1);
    char Msg[Msglenth];
    for(int i = 0; i< Msglenth;i++){
        Msg[i] = sRandom[distr(eng)];
    }
    return std::string(Msg);
}

constexpr size_t LogNum = 1000 * 1000;


void logTest(std::string Msg){
    for(int i = 0;i<LogNum;i++){
        LOGD(Msg);
        LOGE(Msg);
        LOGI(Msg);
        LOGT(Msg);
        LOGW(Msg);
    }
}

void calculPerformance(int threadum){
    std::cout<< "*************************************************************************************"<<std::endl;
    std::vector<std::thread> vThread;
    std::string msg = getTestMessage();
    auto start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < threadum; i++){
        vThread.emplace_back(logTest, msg);
    }
    for(int i = 0; i< threadum;i++){
        vThread[i].join();
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto consume = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    int  num = ((LogNum * 5 * threadum * 1000 )/ consume.count());
    std::cout << "process message :" << LogNum * 5 << " consume time : " << consume.count() << " ms" << std::endl;
    std::cout << "toal" << threadum << " threads " << "average: " << num << " per/seconds" << std::endl;
    std::cout << "per thead average : " << num / 5 << " per/seconds" <<std::endl;
    std::cout<< "*************************************************************************************"<<std::endl;
}

// /**
//     #define __now__ std::chrono::high_resolution_clock::now()
//     using _Ms_ = std::chrono::milliseconds;
//     using _Second_ = std::chrono::seconds;
//     #define SLEEP(Time) std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(Time * 1000)))
//     static inline std::string GETTYPE(_Ms_){ return " ms";}
//     static inline std::string GETTYPE(_Second_){ return " s";}
//     #define _TIMECOUNT_(Function, timeType, Desc) do{\
//                                                     auto start = __now__;\
//                                                     Function;\
//                                                     auto end = __now__;\
//                                                     auto duration = std::chrono::duration_cast<timeType>(end - start);\
//                                                     std::cout << Desc << " cousmeTime: " << duration.count() << yoyo::GETTYPE(timeType()) <<std::endl;\
//                                                     }while(0);   

// */

// void func(){
//     // Logger logger;
//     int num = 1000 * 100;
//     auto start = std::chrono::high_resolution_clock::now();
//     for(int i = 0; i < num; i++){
//         LOGI("This is my  info logger test");
//         LOGE("This is my  info logger test");
//         LOGD("This is my  info logger test");
//         LOGT("This is my  info logger test");
//         LOGF("This is my  info logger test");
//     }
//     auto end = std::chrono::high_resolution_clock::now();
//     auto duration = std::chrono::duration_cast<yoyo::_Ms_>(end - start);
//     std::cout <<"全部进入消息队列: " << duration.count() << yoyo::GETTYPE(yoyo::_Ms_()) <<std::endl;
// }

// void f_sprintf(){
//     int n = 1000000;
//     std::string str;
//     for(int i = 0; i < n; i++){
//         std::string tmp = yoyo::getCurrentTime();
//     }
//     std::ostringstream oss;
//     // oss << str;
//     std::cout << str.size() << std::endl;
// }

// void f_ostream(){
//     int n = 1000000;
//     std::string str;
//     int size = 0;
//     for(int i = 0; i < n; i++){
//         str.append(yoyo::getCurrentTime());
//     }
//     std::ostringstream oss;
//     oss << str;
//     std::cout << str.size() << std::endl;
// }


int main(){
    calculPerformance(1);
    calculPerformance(4);
    calculPerformance(6);
    calculPerformance(8);
    calculPerformance(16);
}
 