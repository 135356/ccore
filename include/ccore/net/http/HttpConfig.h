//http配置
#pragma once
#include <map>
#include <set>
#include <string>
#include "../IpTcpConfig.hpp"
#include "ccore/safe/Log.h"
#include "ccore/File.h"

namespace cc{
    namespace net{
        namespace http{
            //在内存里面存储html
            struct ST_html;
            struct ST_html_arr;
            struct ContentType{
                bool is_gzip{};
                std::string alias{};
            };
            class HttpConfig{
                std::string config_path_ = "./13535_webserve.conf"; //配置文件
                std::map<std::string,std::string> config_info_{}; //配置信息
                HttpConfig();
                ~HttpConfig()=default;
                void getConfigF_(FILE *config_fp);
            public:
                unsigned short PORT_{80}; //服务端口
                int IS_GZIP_=1; //是否支持gzip
                std::string WEB_DIR_ = "html"; //网站目录（相对路径）
                std::set<std::string> ORIGIN_ = {"*"}; //cors跨域白名单
                //文件对应的类型(Content-Type)
                std::map<std::string,ContentType> content_type_{
                    {"aac",{false,"audio/aac"}},
                    {"abw",{false,"application/x-abiword"}},
                    {"arc",{false,"application/x-freearc"}},
                    {"avi",{false,"video/x-msvideo"}},
                    {"azw",{false,"application/vnd.amazon.ebook"}},
                    {"bin",{false,"application/octet-stream"}},
                    {"bmp",{false,"image/bmp"}},
                    {"bz",{false,"application/x-bzip"}},
                    {"bz2",{false,"application/x-bzip2"}},
                    {"csh",{false,"application/x-csh"}},
                    {"css",{true,"text/css"}},
                    {"csv",{true,"text/csv"}},
                    {"doc",{false,"application/msword"}},
                    {"docx",{false,"application/vnd.openxmlformats-officedocument.wordprocessingml.document"}},
                    {"eot",{false,"application/vnd.ms-fontobject"}},
                    {"epub",{false,"application/epub+zip"}},
                    {"gif",{false,"image/gif"}},
                    {"htm",{true,"text/html;charset=utf8"}},
                    {"html",{true,"text/html;charset=utf8"}},
                    {"ico",{false,"image/vnd.microsoft.icon"}},
                    {"ics",{true,"text/calendar"}},
                    {"jar",{true,"application/java-archive"}},
                    {"jpeg",{false,"image/jpeg"}},
                    {"jpg",{false,"image/jpeg"}},
                    {"js",{true,"text/javascript"}},
                    {"json",{true,"application/json"}},
                    {"jsonld",{true,"application/ld+json"}},
                    {"map",{false,"application/json"}},
                    {"mid",{false,"audio/x-midi"}},
                    {"midi",{false,"audio/x-midi"}},
                    {"mjs",{true,"text/javascript"}},
                    {"mp3",{false,"audio/mpeg"}},
                    {"mpeg",{false,"video/mpeg"}},
                    {"mpkg",{true,"application/vnd.apple.installer+xml"}},
                    {"odp",{false,"application/vnd.oasis.opendocument.presentation"}},
                    {"ods",{false,"application/vnd.oasis.opendocument.spreadsheet"}},
                    {"odt",{false,"application/vnd.oasis.opendocument.text"}},
                    {"oga",{false,"audio/ogg"}},
                    {"ogv",{false,"video/ogg"}},
                    {"ogx",{false,"application/ogg"}},
                    {"otf",{true,"font/otf"}},
                    {"png",{false,"image/png"}},
                    {"pdf",{false,"application/pdf"}},
                    {"ppt",{false,"application/vnd.ms-powerpoint"}},
                    {"pptx",{false,"application/vnd.openxmlformats-officedocument.presentationml.presentation"}},
                    {"rar",{false,"application/x-rar-compressed"}},
                    {"rtf",{false,"application/rtf"}},
                    {"sh",{false,"application/x-sh"}},
                    {"svg",{true,"image/svg+xml"}},
                    {"swf",{false,"application/x-shockwave-flash"}},
                    {"tar",{false,"application/x-tar"}},
                    {"tif",{true,"image/tiff"}},
                    {"tiff",{true,"image/tiff"}},
                    {"ttf",{true,"font/ttf"}},
                    {"txt",{true,"text/plain"}},
                    {"vsd",{false,"application/vnd.visio"}},
                    {"wav",{false,"audio/wav"}},
                    {"weba",{false,"audio/webm"}},
                    {"webm",{false,"video/webm"}},
                    {"webp",{false,"image/webp"}},
                    {"woff",{true,"font/woff"}},
                    {"woff2",{true,"font/woff2"}},
                    {"xhtml",{true,"application/xhtml+xml"}},
                    {"xls",{false,"application/vnd.ms-excel"}},
                    {"xlsx",{false,"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"}},
                    {"xml",{true,"text/xml"}},
                    {"xul",{true,"application/vnd.mozilla.xul+xml"}},
                    {"zip",{false,"application/zip"}},
                    {"3gp",{false,"video/3gpp"}},
                    {"3g2",{false,"video/3gpp2"}},
                    {"7z",{false,"application/x-7z-compressed"}},
                    {"default",{false,"text/html;charset=utf8"}}
                };
            public:
                static auto &obj(){
                    static HttpConfig alias;
                    return alias;
                }
            };
        }
    }
}