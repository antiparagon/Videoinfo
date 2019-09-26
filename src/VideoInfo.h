//
//  main.cpp
//  VideoInfo
//
//  Created by Antiparagon on 07/29/19.
//  Copyright © 2019 Antiparagon. All rights reserved.
//
#pragma once

#include <string>
#include <cstdint>
#include <vector>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/log.h>
}

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>

//#include <ffmpeg/swscale.h>

namespace videoinfo
{
    struct StreamInfo
    {
        int64_t start_time = 0;
        int64_t duration = 0;
        int time_base_num = 1;
        int time_base_den = 1;
        int frame_rate_num = 1;
        int frame_rate_den = 1;
        int width = -1;
        int height = -1;
        int channels = -1;
        int sample_rate = -1;
        AVMediaType codec_type = AVMEDIA_TYPE_UNKNOWN;
        std::string codec_name = "";
        int codec_id = -1;
        int64_t bit_rate = 0;
    };
    
    
    struct VideoInfo
    {
        std::string file_path = "";
        std::string file_name = "";
        std::string file_ext = "";
        std::string iformat_name = "";
        int64_t duration = -1;
        int64_t bit_rate = 0;
        
        unsigned int nb_streams = 0;
        std::vector<StreamInfo> streams;
        
        static void save_video_thumbnail(const VideoInfo &video_info, const std::string &thumbnail_path, int width, int height);
        
        static VideoInfo get_video_info(const boost::filesystem::path &video_path);
        static void output_txt_video_report(const std::vector<VideoInfo> &videoinfos, std::ostream &out);
        static void output_html_video_report(const std::vector<VideoInfo> &videoinfos, std::ostream &out);
        static std::vector<boost::filesystem::path> get_video_paths(const std::string &folder, bool recurse = false);
        
        static void save_gray_frame(unsigned char *buf, int wrap, int xsize, int ysize, const std::string& frame_filename);
        static void save_color_frame(AVFrame *pRGBFrame, int width, int height, const std::string& frame_filename);
        static int decode_packet(AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame);
        
        static cv::Mat get_first_frame_mat(const VideoInfo &video_info, int width, int height);
    };

}
