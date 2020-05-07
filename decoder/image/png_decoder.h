//
//  png_decoder.h
//  DTLiving
//
//  Created by Dan Jiang on 2020/4/29.
//  Copyright © 2020 Dan Thought Studio. All rights reserved.
//

#ifndef DTLIVING_DECODER_IMAGE_PNG_DECODER_H_
#define DTLIVING_DECODER_IMAGE_PNG_DECODER_H_

#include <string>

namespace dtliving {
namespace decoder {
namespace image {

typedef struct {
    const int width;
    const int height;
    const void  *data;
} RawImageData;

class PngDecoder {
public:
    PngDecoder();
    ~PngDecoder();

    RawImageData ReadImage(std::string file);
    void DeleteImage();
    
private:
    char *compressed_data_;
};

}
}
}

#endif /* png_decoder_h */
