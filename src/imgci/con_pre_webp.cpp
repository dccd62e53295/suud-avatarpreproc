
#include <string>
#include <vector>
#include <webp/decode.h>
#include <webp/encode.h>

#include "handler_list.hpp"

std::string controller_preview_webp(std::string const& original) {
    // Decode the input WebP image from memory
    int width, height;
    uint8_t* decoded_data = WebPDecodeRGBA(reinterpret_cast<const uint8_t*>(original.data()), original.size(), &width, &height);
    if (nullptr==decoded_data) {
        //throw std::runtime_error("Failed to decode WebP image.");
        return "";
    }

    // Define the target size for the thumbnail
    int target_width = width / 2; // Adjust the scaling factor as needed
    int target_height = height / 2;

    // Resize the image (using a simple nearest-neighbor scaling for demonstration)
    std::vector<uint8_t> resized_data(target_width * target_height * 4);
    for (int y = 0; y < target_height; ++y) {
        for (int x = 0; x < target_width; ++x) {
            int src_x = x * 2;
            int src_y = y * 2;
            int src_index = (src_y * width + src_x) * 4;
            int dst_index = (y * target_width + x) * 4;
            for (int i = 0; i < 4; ++i) {
                resized_data[dst_index + i] = decoded_data[src_index + i];
            }
        }
    }

    // Encode the resized image to WebP format
    uint8_t* output_data = nullptr;
    size_t output_size = WebPEncodeRGBA(resized_data.data(), target_width, target_height, target_width * 4, 90, &output_data);
    if (output_size == 0) {
        WebPFree(decoded_data);
        //throw std::runtime_error("Failed to encode WebP image.");
        return "";
    }

    // Check the output size and adjust quality factor if necessary
    int quality_factor = 66;
    while (output_size > 1024 * 1024) {
        WebPFree(output_data);
        quality_factor -=33;
        if (quality_factor <= 0) {
            WebPFree(decoded_data);
            //throw std::runtime_error("Unable to reduce image size below 1MB.");
            return "";
        }
        output_size = WebPEncodeRGBA(resized_data.data(), target_width, target_height, target_width * 4, quality_factor, &output_data);
    }

    // Convert the output data to std::string
    std::string thumbnail(reinterpret_cast<char*>(output_data), output_size);

    // Clean up
    WebPFree(decoded_data);
    WebPFree(output_data);

    return thumbnail;
};
