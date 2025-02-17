//
// Created by ender on 25-2-17.
//

#ifndef FRAME_HPP
#define FRAME_HPP

#include <memory>
#include <opencv2/opencv.hpp>

extern "C" {
#include <libavutil/frame.h>
#include <libswscale/swscale.h>
}

namespace scrcpy {
    class frame : public std::enable_shared_from_this<frame> {
    public:
        explicit frame(AVFrame *raw_frame);

        static auto create_shared(AVFrame *raw_frame) -> std::shared_ptr<frame>;

        ~frame();

        auto raw() const -> AVFrame *;

        auto mat() const -> std::shared_ptr<cv::Mat>;

    private:
        AVFrame *raw_frame = nullptr;
    };
}


#endif //FRAME_HPP
