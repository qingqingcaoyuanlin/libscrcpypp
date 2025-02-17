//
// Created by ender on 25-2-16.
//

#ifndef DECODER_HPP
#define DECODER_HPP


#include <frame.hpp>
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

namespace scrcpy {
    class h264_decoder {
    public:
        h264_decoder();

        ~h264_decoder();

        std::vector<std::shared_ptr<frame> > decode(AVPacket *packet) const;

    private:
        const AVCodec *codec = nullptr;
        AVCodecContext *codec_ctx = nullptr;
    };
}

#endif //DECODER_HPP
