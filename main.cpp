#include <iostream>

#include "png_decoder.h"

using namespace dtliving;

int main() {
    decoder::image::PngDecoder *png_decoder;
    decoder::image::RawImageData image_data = png_decoder->ReadImage("/Users/danjiang/Downloads/WID-small.png");
    std::cout << image_data.width << "," << image_data.height << std::endl;
    return 0;
}
