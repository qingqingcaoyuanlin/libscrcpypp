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

    auto frame::pix_fmt() const -> AVPixelFormat {
        return static_cast<AVPixelFormat>(this->raw_frame->format);
    }

    auto frame::raw() const -> AVFrame * {
        return this->raw_frame;
    }

    auto frame::to_rgba() const -> std::shared_ptr<frame> {
        auto new_frame = av_frame_alloc();
        const auto w = this->raw_frame->width;
        const auto h = this->raw_frame->height;
        SwsContext *conversion = sws_getContext(w, h,
                                                static_cast<AVPixelFormat>(raw_frame->format),
                                                w,
                                                h,
                                                AV_PIX_FMT_RGBA, SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
        new_frame->format = AV_PIX_FMT_RGBA;
        new_frame->width = w;
        new_frame->height = h;
        if (av_frame_get_buffer(new_frame, 32) < 0) {
            av_frame_free(&new_frame);
            throw std::runtime_error("Failed to allocate output frame buffers");
        }
        sws_scale(conversion, raw_frame->data, raw_frame->linesize, 0, h, new_frame->data, new_frame->linesize);
        sws_freeContext(conversion);
        av_frame_copy_props(new_frame, this->raw_frame);
        return std::make_shared<frame>(new_frame);
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
