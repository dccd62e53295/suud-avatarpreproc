
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

std::unordered_map<std::string,std::function<void(i_scgi_r&)>,StringHasher,StringComparator> route_known_handlers={};
// https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types#image_types
void* i_scgi_obj_init(std::string_view){
route_known_handlers.insert_or_assign("image/gif",controller_preview_gif);
route_known_handlers.insert_or_assign("image/webp",controller_preview_webp);

return nullptr;
};

void i_scgi_handle_req(i_scgi_r& p1){
auto v1=route_known_handlers.find(p1.req_content_type);
    if (v1 != route_known_handlers.end()) {
        v1->second(p1);
    } else {
        // nothing to do
        p1.processed=false;
    }
};
