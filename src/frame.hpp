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

        /**
         * Get frame pixel format.
         * @return A ffmpeg AVPixelFormat enum represents its internal raw frame format.
         */
        auto pix_fmt() const -> AVPixelFormat;

        /**
         * Get yuv420p raw ffmpeg AVFrame.
         * @return A yuv420p pix fmt AVFrame, managed by std::shared_ptr of this frame object.
         */
        auto raw() const -> AVFrame *;

        /**
         * Make a new frame obj and convert its pixel format to BGRA. This method is provided for render usages.
         * @return A new BGRA frame obj.
         */
        auto to_bgra() const -> std::shared_ptr<frame>;

        /**
         * Get rgb24 opencv mat.
         * @return A OpenCV RGB24 pix fmt Mat, managed by std::shared_ptr.
         */
        auto mat() const -> std::shared_ptr<cv::Mat>;

    private:
        AVFrame *raw_frame = nullptr;
    };
}


#endif //FRAME_HPP
