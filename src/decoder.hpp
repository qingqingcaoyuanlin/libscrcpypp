//
// Created by ender on 25-2-16.
//

#ifndef DECODER_HPP
#define DECODER_HPP


#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

class h264_decoder {
public:
    h264_decoder() {
        codec = avcodec_find_decoder(AV_CODEC_ID_H264);
        if (!codec) throw std::runtime_error("H.264 codec not found");

        codec_ctx = avcodec_alloc_context3(codec);
        if (!codec_ctx) throw std::runtime_error("Could not allocate codec context");

        if (avcodec_open2(codec_ctx, codec, nullptr) < 0)
            throw std::runtime_error("Could not open codec");
    }

    ~h264_decoder() {
        avcodec_free_context(&codec_ctx);
    }

    [[nodiscard]] std::vector<AVFrame *> decode(AVPacket *packet) const {
        if (packet->pts == AV_NOPTS_VALUE) {
            return {};
        }
        if (avcodec_send_packet(codec_ctx, packet) != 0) {
            av_packet_free(&packet);
            return {};
        }
        av_packet_free(&packet);

        std::vector<AVFrame *> frames;
        AVFrame *frame = av_frame_alloc();
        while (true) {
            const int ret = avcodec_receive_frame(codec_ctx, frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
            if (ret < 0) break;

            AVFrame *cloned = av_frame_clone(frame);
            frames.push_back(cloned);
        }
        av_frame_free(&frame);
        return frames;
    }

    static auto avframe_to_mat(const AVFrame *frame) -> cv::Mat {
        const auto width = frame->width;
        const auto height = frame->height;
        cv::Mat image(height, width, CV_8UC3);
        std::array<std::int32_t, 1> cv_lines_size{};
        cv_lines_size.at(0) = static_cast<std::int32_t>(image.step1());
        SwsContext *conversion = sws_getContext(width, height, static_cast<AVPixelFormat>(frame->format), width, height,
                                                AV_PIX_FMT_BGR24, SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
        sws_scale(conversion, frame->data, frame->linesize, 0, height, &image.data, cv_lines_size.data());
        sws_freeContext(conversion);
        return image;
    }

private:
    const AVCodec *codec = nullptr;
    AVCodecContext *codec_ctx = nullptr;
};


#endif //DECODER_HPP
