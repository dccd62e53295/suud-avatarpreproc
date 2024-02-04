
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
std::string inline sha3_256(const char* input_p,size_t input_l)
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
}

bool AvatarProcess::run(){
boost::gil::rgba8_image_t v1;
{
	std::stringstream v3(std::ios_base::in|std::ios_base::out|std::ios_base::binary);
v3.write(this->in.c_str(),this->in.size());
//https://stackoverflow.com/questions/16259659/read-a-jpeg-image-from-memory-with-boostgil
//https://stackoverflow.com/questions/61656110/how-to-read-image-info-from-python-bytes-string-using-boostgil
//https://www.boost.org/doc/libs/1_81_0/libs/gil/doc/html/io.html#reading-and-writing-in-memory-buffers

//boost::gil::rgba8_image_t v7;
//boost::gil::image_read_settings<png_tag> v6;
boost::gil::read_image(v3,v1,boost::gil::png_tag());
}

auto v4=view(v1);

if(v4.width()!=512){
this->error=3;
return false;
}

if(v4.height()!=512){
this->error=4;
return false;
}

//https://www.boost.org/doc/libs/1_80_0/libs/gil/doc/html/design/image_view.html
//https://www.boost.org/doc/libs/1_80_0/libs/gil/doc/html/tutorial/gradient.html
//https://www.boost.org/doc/libs/1_68_0/libs/gil/doc/html/io.html
boost::gil::rgba8_image_t v6(512,512);
auto v7=view(v6);
boost::gil::copy_pixels(v4,v7);

/*{
char* v8 = reinterpret_cast<char*>(boost::gil::interleaved_view_get_raw_data(v7));
this->hash=sha3_256(v8,4*512*512);
}*/
{
	uint32_t alpha_sum=0;
	uint8_t hash_dup[4];

	  uint32_t digest_length = SHA256_DIGEST_LENGTH;
	  const EVP_MD* algorithm = EVP_sha3_256();
	  uint8_t* digest = static_cast<uint8_t*>(OPENSSL_malloc(digest_length));
	  EVP_MD_CTX* context = EVP_MD_CTX_new();
	  EVP_DigestInit_ex(context, algorithm, nullptr);
	for (int y = 0; y < 512; ++y) {
		for (int x = 0; x < 512; ++x) {
			boost::gil::rgba8_pixel_t& pixel = v7(x, y);
			uint8_t r = boost::gil::get_color(pixel, boost::gil::red_t());
			uint8_t g = boost::gil::get_color(pixel, boost::gil::green_t());
			uint8_t b = boost::gil::get_color(pixel, boost::gil::blue_t());
			uint8_t a = boost::gil::get_color(pixel, boost::gil::alpha_t());

			alpha_sum+=a;
			hash_dup[0]=r;
			hash_dup[1]=g;
			hash_dup[2]=b;
			hash_dup[3]=a;

			EVP_DigestUpdate(context,hash_dup,4);
		}
	}
	  EVP_DigestFinal_ex(context, digest, &digest_length);
	  EVP_MD_CTX_destroy(context);
	  std::string output(reinterpret_cast<char*>(digest), digest_length);
	  OPENSSL_free(digest);
	  //return output;
	  if(alpha_sum<(512*512*256/4)){
		  this->error=2;
		  return false;
	  }
	  this->hash=output;
}

struct boost::gil::image_write_info<boost::gil::png_tag> ve;
ve._compression_level=9;
ve._compression_mem_level=MAX_MEM_LEVEL;
ve._compression_strategy=Z_FILTERED;
ve._compression_window_bits=15;
ve._compression_method=8;
ve._compression_buffer_size=8192;
std::stringstream v8(std::ios_base::in|std::ios_base::out|std::ios_base::binary);
boost::gil::write_view(v8,v7,ve);
this->out=v8.str();

this->error=0;
return true;
};







