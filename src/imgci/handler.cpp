
#include <cstring>
#include <iostream>
#include <unordered_map>
#include <functional>

#include "route.hpp"
#include "handler_list.hpp"

struct StringHasher {
    inline std::size_t operator()(const std::string& str) const {
        return std::hash<std::string>{}(str);
    }
};

struct StringComparator {
    inline bool operator()(const std::string& lhs, const std::string& rhs) const {
        return lhs == rhs;
    }
};

std::unordered_map<std::string,std::function<std::string(std::string const&)>,StringHasher,StringComparator> route_known_handlers={};
// https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types#image_types
void* i_scgi_obj_init(std::string_view){
route_known_handlers.insert_or_assign("image/gif",controller_preview_gif);
route_known_handlers.insert_or_assign("image/webp",controller_preview_webp);
route_known_handlers.insert_or_assign("image/jpeg",controller_preview_jpg);
route_known_handlers.insert_or_assign("image/png",controller_preview_png);
return nullptr;
};

void i_scgi_handle_req(i_scgi_r& p1){
p1.processed=false;
auto v1=route_known_handlers.find(p1.req_content_type);
if (v1 != route_known_handlers.end()) {
std::string v2=v1->second(p1.req_data);
if(!v2.empty()){
p1.res_data=v2;
p1.processed=true;
}
}
};
