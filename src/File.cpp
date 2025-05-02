#include "ccore/File.h"

namespace cc{
    size_t File::mkDir(const std::string &dir){
        std::vector<std::string> dir_list{};
        std::string str;
        for(auto &v:dir){
            if(v == '.'){}else if(v == '/'){
                if(!str.empty()){
                    dir_list.push_back(str);
                    str += '/';
                }
            }else{
                str+=v;
            }
        }
        if(!str.empty()){
            dir_list.push_back(str);
        }
        for(auto &v:dir_list){
            if(access(("./"+v).c_str(),F_OK) == -1){
                mkdir(("./"+v).c_str(),S_IRWXU);
            }
        }
        return dir_list.size();
    }
    size_t File::mkFile(const std::string &dir,const std::string &name,const std::string &form,std::string &path){
        //如果目录不存在,就递归创建目录
        File::mkDir(dir);
        //判断是否有重名文件
        size_t index{};
        std::string file_path = dir+'/'+name;
        while(true){
            path = file_path;
            if(index){
                path += '_'+std::to_string(index);
            }
            path += form;
            if(access(path.c_str(),F_OK) == -1){break;} //不存在
            index++;
        }
        return index;
    }
}