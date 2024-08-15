
#include "avatar_proc.hpp"

#include <sstream>
#include <vector>

#include <boost/gil.hpp>
#include <boost/gil/io/io.hpp>
#include <boost/gil/extension/io/png.hpp>

//#include <boost/gil/extension/io/base64_io.hpp>
//#include <boost/gil/extension/hash/hash.hpp>
//#include <boost/gil/extension/hash/sha3.hpp>

//#include "context.hpp"
#include <openssl/evp.h>
#include <openssl/sha.h>

//perform the SHA3-256 hash
inline std::string sha3_256(const char* input_p,size_t input_l)
{
  uint32_t digest_length = SHA256_DIGEST_LENGTH;
  const EVP_MD* algorithm = EVP_sha3_256();
  uint8_t* digest = static_cast<uint8_t*>(OPENSSL_malloc(digest_length));
  EVP_MD_CTX* context = EVP_MD_CTX_new();
  EVP_DigestInit_ex(context, algorithm, nullptr);
  EVP_DigestUpdate(context, input_p, input_l);
  EVP_DigestFinal_ex(context, digest, &digest_length);
  EVP_MD_CTX_destroy(context);
  std::string output(reinterpret_cast<char*>(digest), digest_length);
  OPENSSL_free(digest);
  return output;
};

inline void process_rgb(AvatarProcess* v0,std::stringstream& v3,boost::gil::rgba8_view_t& v7){
boost::gil::rgb8_image_t v1;
//boost::gil::image_read_settings<png_tag> v6;
boost::gil::read_image(v3,v1,boost::gil::png_tag());

auto v4=boost::gil::view(v1);

	  uint32_t digest_length = SHA256_DIGEST_LENGTH;
	  const EVP_MD* algorithm = EVP_sha3_256();
	  uint8_t* digest = static_cast<uint8_t*>(OPENSSL_malloc(digest_length));
	  EVP_MD_CTX* context = EVP_MD_CTX_new();
	  EVP_DigestInit_ex(context, algorithm, nullptr);
	for (int y = 0; y < 512; ++y) {
		uint8_t hash_dup[512*4];
		memset(hash_dup, 0xff, sizeof(hash_dup));
		for (int x = 0; x < 512; ++x) {
			boost::gil::rgb8_pixel_t const& in_pixel = v4(x, y);
			uint8_t r = boost::gil::get_color(in_pixel, boost::gil::red_t());
			uint8_t g = boost::gil::get_color(in_pixel, boost::gil::green_t());
			uint8_t b = boost::gil::get_color(in_pixel, boost::gil::blue_t());

			boost::gil::rgba8_pixel_t& out_pixel = v7(x, y);
			boost::gil::at_c<0>(out_pixel) = r; // Red channel
    		boost::gil::at_c<1>(out_pixel) = g; // Green channel
    		boost::gil::at_c<2>(out_pixel) = b; // Blue channel
    		boost::gil::at_c<3>(out_pixel) = 0xff; 

			uint8_t hash_dup_offset=x*4;
			hash_dup[hash_dup_offset|0]=r;
			hash_dup[hash_dup_offset|1]=g;
			hash_dup[hash_dup_offset|2]=b;
		}
		EVP_DigestUpdate(context,hash_dup,512*4);
	}
	  EVP_DigestFinal_ex(context, digest, &digest_length);
	  EVP_MD_CTX_destroy(context);
	  std::string output(reinterpret_cast<char*>(digest), digest_length);
	  OPENSSL_free(digest);
	  v0->hash=output;
};

inline void process_rgba(AvatarProcess* v0,std::stringstream& v3,boost::gil::rgba8_view_t& v7){
boost::gil::rgba8_image_t v1;
//boost::gil::image_read_settings<png_tag> v6;
boost::gil::read_image(v3,v1,boost::gil::png_tag());

auto v4=boost::gil::view(v1);

	  uint32_t digest_length = SHA256_DIGEST_LENGTH;
	  const EVP_MD* algorithm = EVP_sha3_256();
	  uint8_t* digest = static_cast<uint8_t*>(OPENSSL_malloc(digest_length));
	  EVP_MD_CTX* context = EVP_MD_CTX_new();
	  EVP_DigestInit_ex(context, algorithm, nullptr);
	for (int y = 0; y < 512; ++y) {
		uint8_t hash_dup[512*4];
		for (int x = 0; x < 512; ++x) {
			boost::gil::rgba8_pixel_t const& in_pixel = v4(x, y);
			uint8_t r = boost::gil::get_color(in_pixel, boost::gil::red_t());
			uint8_t g = boost::gil::get_color(in_pixel, boost::gil::green_t());
			uint8_t b = boost::gil::get_color(in_pixel, boost::gil::blue_t());
			uint8_t a = boost::gil::get_color(in_pixel, boost::gil::alpha_t());

			boost::gil::rgba8_pixel_t& out_pixel = v7(x, y);
			boost::gil::at_c<0>(out_pixel) = r; // Red channel
    		boost::gil::at_c<1>(out_pixel) = g; // Green channel
    		boost::gil::at_c<2>(out_pixel) = b; // Blue channel
    		boost::gil::at_c<3>(out_pixel) = a; 

			uint8_t hash_dup_offset=x*4;
			hash_dup[hash_dup_offset|0]=r;
			hash_dup[hash_dup_offset|1]=g;
			hash_dup[hash_dup_offset|2]=b;
			hash_dup[hash_dup_offset|3]=a;
		}
		EVP_DigestUpdate(context,hash_dup,512*4);
	}
	  EVP_DigestFinal_ex(context, digest, &digest_length);
	  EVP_MD_CTX_destroy(context);
	  std::string output(reinterpret_cast<char*>(digest), digest_length);
	  OPENSSL_free(digest);
	  v0->hash=output;
};

bool AvatarProcess::run(){
//https://stackoverflow.com/questions/16259659/read-a-jpeg-image-from-memory-with-boostgil
//https://stackoverflow.com/questions/61656110/how-to-read-image-info-from-python-bytes-string-using-boostgil
//https://www.boost.org/doc/libs/1_81_0/libs/gil/doc/html/io.html#reading-and-writing-in-memory-buffers
std::stringstream v1(this->in,std::ios::in|std::ios::out|std::ios::binary);
v1.clear();
v1.seekg(0,std::ios::beg);
//v1.write(this->in.c_str(),this->in.size());

std::size_t v2=0;
{
boost::gil::any_image<boost::gil::rgba8_image_t,boost::gil::rgb8_image_t> v3;
boost::gil::read_image(v1,v3,boost::gil::png_tag());
v1.clear();
v1.seekg(0,std::ios::beg);

if(v3.height()!=512){
this->error=3;
return false;
}

if(v3.width()!=512){
this->error=4;
return false;
}

v2=v3.num_channels();
}
//https://www.boost.org/doc/libs/1_80_0/libs/gil/doc/html/design/image_view.html
//https://www.boost.org/doc/libs/1_80_0/libs/gil/doc/html/tutorial/gradient.html
//https://www.boost.org/doc/libs/1_68_0/libs/gil/doc/html/io.html
boost::gil::rgba8_image_t v3(512,512);
auto v4=boost::gil::view(v3);

switch(v2){
case 4:
{
process_rgba(this,v1,v4);
}
break;
case 3:
{
process_rgb(this,v1,v4);
}
break;
default:
{
this->error=5;
return false;
}
break;
}

struct boost::gil::image_write_info<boost::gil::png_tag> v6;
v6._compression_level=9;
v6._compression_mem_level=MAX_MEM_LEVEL;
v6._compression_strategy=Z_FILTERED;
v6._compression_window_bits=15;
v6._compression_method=8;
v6._compression_buffer_size=8192;
std::stringstream v7(std::ios_base::in|std::ios_base::out|std::ios_base::binary);
boost::gil::write_view(v7,v4,v6);
this->out=v7.str();

this->error=0;
return true;
};
