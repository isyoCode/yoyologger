
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
    std::cout << "toal " << threadum << " threads " << "average: " << num << " per/seconds" << std::endl;
    std::cout << "per thead average : " << num / 5 << " per/seconds" <<std::endl;
    std::cout<< "*************************************************************************************"<<std::endl;
}



int main(){
    calculPerformance(1);
    calculPerformance(4);
    calculPerformance(6);
    calculPerformance(8);
    calculPerformance(16);
}
 
