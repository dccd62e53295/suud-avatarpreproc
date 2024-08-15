#include <sstream>
#include <vector>

#include <boost/gil.hpp>
#include <boost/gil/io/io.hpp>
#include <boost/gil/extension/io/png.hpp>

#include <webp/encode.h>

#include "handler_list.hpp"

inline void process_rgba(std::stringstream& p1,std::vector<uint8_t>& p2){
boost::gil::rgba8_image_t v1;
boost::gil::read_image(p1,v1,boost::gil::png_tag());

auto v2 = boost::gil::view(v1);
auto v3 = p2.begin();
    for (auto y = 0; y < v2.height(); ++y) {
        for (auto x = 0; x < v2.width(); ++x) {
			boost::gil::rgba8_pixel_t v4=v2(x, y);
            *v3++ = v4[0]; // R
            *v3++ = v4[1]; // G
            *v3++ = v4[2]; // B
            *v3++ = v4[3]; // A
        }
    }
};

inline void process_rgb(std::stringstream& p1,std::vector<uint8_t>& p2){
boost::gil::rgb8_image_t v1;
boost::gil::read_image(p1,v1,boost::gil::png_tag());

auto v2 = boost::gil::view(v1);
auto v3 = p2.begin();
    for (auto y = 0; y < v2.height(); ++y) {
        for (auto x = 0; x < v2.width(); ++x) {
			boost::gil::rgb8_pixel_t v4=v2(x, y);
            *v3++ = v4[0]; // R
            *v3++ = v4[1]; // G
            *v3++ = v4[2]; // B
            *v3++ = 0xff; // A
        }
    }
};

std::string controller_preview_png(std::string const& p1){
std::stringstream v1(p1,std::ios_base::in|std::ios_base::out|std::ios_base::binary);
v1.clear();
v1.seekg(0,std::ios::beg);
struct{
std::size_t num_channels;
std::size_t height;
std::size_t width;
} v2;
{
boost::gil::any_image<boost::gil::rgba8_image_t,boost::gil::rgb8_image_t> v3;
boost::gil::read_image(v1,v3,boost::gil::png_tag());
v2.num_channels=v3.num_channels();
v2.height=v3.height();
v2.width=v3.width();
}
v1.clear();
v1.seekg(0,std::ios::beg);
/*
y <= height
x <= width
webp_index = y * width + x
*/
std::vector<uint8_t> v4(v2.height * v2.width * 4);
switch(v2.num_channels){
case 4:
{
process_rgba(v1,v4);
}
break;
case 3:
{
process_rgb(v1,v4);
}
break;
default:
{
return "";
}
break;
}
    // Step 2: Encode raw RGBA data to WebP format using libwebp
    uint8_t* output_data = nullptr;
    size_t output_size = WebPEncodeRGBA(v4.data(), v2.width, v2.height, v2.width * 4, 66, &output_data);
    if (output_size == 0) {
        //throw std::runtime_error("Error encoding image to WebP format");
        return "";
    }

    // Step 3: Store the encoded WebP data in a std::string
    std::string webp_string(reinterpret_cast<char*>(output_data), output_size);
    
    // Free the output data allocated by WebPEncodeRGBA
    WebPFree(output_data);

    return webp_string;
};
