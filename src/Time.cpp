// Created by 邦邦 on 2022/6/17.
#include "ccore/Time.h"

void cc::Time::sleep(const int &second){
    std::this_thread::sleep_for(std::chrono::seconds(second));
}
time_t cc::Time::getTime(){
    return time(nullptr);
}
time_t cc::Time::getTimeMs(){
    return duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
std::string cc::Time::getDate(const char format[],const time_t &target_time){
    char date[32]={};
    strftime(date, sizeof(date),format,localtime(&target_time));
    return date;
}
std::string cc::Time::getDateAuto(const time_t &target_time){
    std::string date;
    time_t time_current = time(nullptr);
    struct tm p_compare{};
    if(target_time > 0){
        p_compare = *gmtime(&target_time);
        uint32_t target_time_day = (uint32_t)target_time/86400;
        uint32_t time_current_day = (uint32_t)time_current/86400;
        if(target_time_day == time_current_day){
            date = "今天";
        }else if(target_time_day == time_current_day-1){
            date = "昨天";
        }else{ //(2022年10月10日)
            date = std::to_string(1900+p_compare.tm_year)+"年";
            date += std::to_string(1+p_compare.tm_mon)+"月";
            date += std::to_string(p_compare.tm_mday)+"日";
        }
    }else{
        p_compare = *gmtime(&time_current);
        date = "今天";
    }
    date += ' '+std::to_string(8+p_compare.tm_hour)+":";
    date += p_compare.tm_min<10?'0'+std::to_string(p_compare.tm_min):std::to_string(p_compare.tm_min);
    return date;
}
