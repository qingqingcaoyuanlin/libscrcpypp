//
// Created by ender on 25-2-17.
//

#include "frame.hpp"

namespace scrcpy {
    frame::frame(AVFrame *raw_frame): raw_frame(raw_frame) {
    }

    auto frame::create_shared(AVFrame *raw_frame) -> std::shared_ptr<frame> {
        return std::make_shared<frame>(raw_frame);
    }

    frame::~frame() {
        av_frame_free(&this->raw_frame);
    }

    auto frame::raw() const -> AVFrame * {
        return this->raw_frame;
    }

    [[nodiscard]] auto frame::rgb24_frame() const -> AVFrame * {
        SwsContext *sws_ctx = sws_getContext(
        raw_frame->width, raw_frame->height, static_cast<AVPixelFormat>(raw_frame->format),
        raw_frame->width, raw_frame->height, AV_PIX_FMT_RGB24,
        SWS_FAST_BILINEAR | SWS_ACCURATE_RND,
        nullptr, nullptr, nullptr
    );
        AVFrame *rgb_frame = av_frame_alloc();
        rgb_frame->format = AV_PIX_FMT_RGB24;
        rgb_frame->width = raw_frame->width;
        rgb_frame->height = raw_frame->height;
        av_frame_get_buffer(rgb_frame, 0);
        sws_scale(sws_ctx, raw_frame->data, raw_frame->linesize, 0,
              raw_frame->height, rgb_frame->data, rgb_frame->linesize);
        sws_freeContext(sws_ctx);
        return rgb_frame;
    }

    auto frame::mat() const -> std::shared_ptr<cv::Mat> {
        const auto width = raw_frame->width;
        const auto height = raw_frame->height;
        auto image = std::make_shared<cv::Mat>(height, width, CV_8UC3);
        const std::array cv_lines_size{static_cast<std::int32_t>(image->step1()), 0, 0, 0};
        SwsContext *conversion = sws_getContext(width, height, static_cast<AVPixelFormat>(raw_frame->format), width,
                                                height,
                                                AV_PIX_FMT_RGB24, SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
        sws_scale(conversion, raw_frame->data, raw_frame->linesize, 0, height, &image->data, cv_lines_size.data());
        sws_freeContext(conversion);
        return image;
    }
}
