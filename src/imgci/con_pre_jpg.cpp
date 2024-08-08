
#include <sstream>
#include <vector>

#include <boost/gil.hpp>
#include <boost/gil/io/io.hpp>
#include <boost/gil/extension/io/jpeg.hpp>

#include <webp/encode.h>

#include "handler_list.hpp"

std::string controller_preview_jpg(std::string const& p1){
boost::gil::rgb8_image_t v1;
{
std::stringstream v3(std::ios_base::in|std::ios_base::out|std::ios_base::binary);
v3.write(p1.c_str(),p1.size());
boost::gil::read_image(v3,v1,boost::gil::jpeg_tag());
}
/*
y <= height
x <= width
webp_index = y * width + x
*/
std::vector<uint8_t> rgb_data(v1.width() * v1.height() * 3);
{
// Step 1: Convert boost::gil::rgb8_image_t to raw RGB data
auto view = boost::gil::view(v1);
    
    auto it = rgb_data.begin();
    for (auto y = 0; y < view.height(); ++y) {
        for (auto x = 0; x < view.width(); ++x) {
            *it++ = view(x, y)[0]; // R
            *it++ = view(x, y)[1]; // G
            *it++ = view(x, y)[2]; // B
        }
    }
}
    // Step 2: Encode raw RGB data to WebP format using libwebp
    uint8_t* output_data = nullptr;
    size_t output_size = WebPEncodeRGB(rgb_data.data(), v1.width(), v1.height(), v1.width() * 3, 66, &output_data);
    if (output_size == 0) {
        //throw std::runtime_error("Error encoding image to WebP format");
        return "";
    }

    // Step 3: Store the encoded WebP data in a std::string
    std::string webp_string(reinterpret_cast<char*>(output_data), output_size);
    
    // Free the output data allocated by WebPEncodeRGB
    WebPFree(output_data);

    return webp_string;
};
