//
// Created by ender on 25-2-16.
//

#include "decoder.hpp"

namespace scrcpy {
    h264_decoder::h264_decoder() {
        codec = avcodec_find_decoder(AV_CODEC_ID_H264);
        if (!codec) throw std::runtime_error("H.264 codec not found");

        codec_ctx = avcodec_alloc_context3(codec);
        if (!codec_ctx) throw std::runtime_error("Could not allocate codec context");

        if (avcodec_open2(codec_ctx, codec, nullptr) < 0)
            throw std::runtime_error("Could not open codec");
    }

    h264_decoder::~h264_decoder() {
        avcodec_free_context(&codec_ctx);
    }

    std::vector<std::shared_ptr<frame> > h264_decoder::decode(AVPacket *packet) const {
        if (packet->pts == AV_NOPTS_VALUE) {
            return {};
        }
        if (avcodec_send_packet(codec_ctx, packet) != 0) {
            av_packet_free(&packet);
            return {};
        }
        av_packet_free(&packet);

        std::vector<std::shared_ptr<frame> > frames;
        AVFrame *frame = av_frame_alloc();
        while (true) {
            const int ret = avcodec_receive_frame(codec_ctx, frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
            if (ret < 0) break;

            AVFrame *cloned = av_frame_clone(frame);
            frames.push_back(frame::create_shared(cloned));
        }
        av_frame_free(&frame);
        return frames;
    }
}
