//
//  VideoInfo.cpp
//  videoinfo
//
//  Created by Antiparagon on 8/2/19.
//
#include "VideoInfo.h"

#include <string>
#include <iostream>
#include <regex>
#include <vector>

using namespace boost::algorithm;
using namespace boost::filesystem;
using namespace videoinfo;

uint64_t VideoInfo::get_frame_count(const VideoInfo &video_info)
{
    uint64_t frames = 0;
    
    int curr_log_level = av_log_get_level();
    av_log_set_level(AV_LOG_QUIET);
    
    AVFormatContext *pFormatContext = avformat_alloc_context();
    if (!pFormatContext)
    {
        std::cerr << "ERROR could not allocate memory for Format Context" << '\n';
        return frames;
    }
    
    if (avformat_open_input(&pFormatContext, video_info.file_path.c_str(), NULL, NULL) != 0)
    {
        std::cerr << "ERROR could not open the file: " << video_info.file_path << '\n';
        return frames;
    }
    
    if (avformat_find_stream_info(pFormatContext,  NULL) < 0)
    {
        std::cerr << "ERROR could not get the stream info" << '\n';
        return frames;
    }
    
    AVCodec *pCodec = nullptr;
    AVCodecParameters *pCodecParameters =  nullptr;
    int video_stream_index = -1;
    
    ///////////////////////////////////////////////////////////////////
    
    for (int i = 0; i < pFormatContext->nb_streams; i++)
    {
        AVCodecParameters *pLocalCodecParameters =  NULL;
        pLocalCodecParameters = pFormatContext->streams[i]->codecpar;
        
        AVCodec *pLocalCodec = nullptr;
        
        pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);
        
        if(pLocalCodec == nullptr)
        {
            std::cerr << "ERROR unsupported codec!" << '\n';
            continue;
        }
        
        // when the stream is a video we store its index, codec parameters and codec
        if(pLocalCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            if (video_stream_index == -1)
            {
                video_stream_index = i;
                pCodec = pLocalCodec;
                pCodecParameters = pLocalCodecParameters;
                break;
            }
        }
    }

    AVCodecContext *pCodecContext = avcodec_alloc_context3(pCodec);
    if (!pCodecContext)
    {
        std::cerr << "ERROR failed to allocated memory for AVCodecContext" << '\n';
        return frames;
    }
    
    if (avcodec_parameters_to_context(pCodecContext, pCodecParameters) < 0)
    {
        std::cerr << "ERROR failed to copy codec params to codec context" << '\n';
        return frames;
    }

    if (avcodec_open2(pCodecContext, pCodec, nullptr) < 0)
    {
        std::cerr << "ERROR failed to open codec through avcodec_open2" << '\n';
        return frames;
    }

    AVFrame *pFrame = av_frame_alloc();
    if (!pFrame)
    {
        std::cerr << "ERROR failed to allocated memory for AVFrame" << '\n';
        return frames;
    }
    
    ////////////////////////////////////////////////
    
    // Allocate a color AVFrame structure
    AVFrame *pFrameRGB = av_frame_alloc();
    if(!pFrameRGB)
    {
        std::cerr << "ERROR failed to allocated memory for RGB AVFrame" << '\n';
        return frames;
    }

    
    /////////////////////////////////////////////////

    AVPacket *pPacket = av_packet_alloc();
    if (!pPacket)
    {
        std::cerr << "ERROR failed to allocated memory for AVPacket" << '\n';
        return frames;
    }
    
    int response = 0;

    while (av_read_frame(pFormatContext, pPacket) >= 0)
    {
      // if it's the video stream
      if (pPacket->stream_index == video_stream_index)
      {
        response = decode_packet(pPacket, pCodecContext, pFrame);
        if(response == 0)
        {
            av_packet_unref(pPacket);
            frames++;
        }
      }
      av_packet_unref(pPacket);
    }
    
    avformat_close_input(&pFormatContext);
    avformat_free_context(pFormatContext);
    av_packet_free(&pPacket);
    av_frame_free(&pFrame);
    av_frame_free(&pFrameRGB);
    avcodec_free_context(&pCodecContext);
    
    ///////////////////////////////////////////////////////////////////
    
    av_log_set_level(curr_log_level);
    
    return frames;
}

void VideoInfo::save_video_thumbnail(const VideoInfo &video_info, const std::string &thumbnail_path, int width, int height)
{
    cv::Mat mat = VideoInfo::get_first_frame_mat(video_info, width, height);
    
    if (mat.empty())
    {
        std::cerr << "Couldn't create Mat for first frame: " << video_info.file_path << '\n';
        return;
    }
    
    cv::imwrite(thumbnail_path, mat);
}

cv::Mat VideoInfo::get_first_frame_mat(const VideoInfo &video_info, int width, int height)
{
    cv::Mat mat;
    
    int curr_log_level = av_log_get_level();
    av_log_set_level(AV_LOG_QUIET);
    
    AVFormatContext *pFormatContext = avformat_alloc_context();
    if (!pFormatContext)
    {
        std::cerr << "ERROR could not allocate memory for Format Context" << '\n';
        return mat;
    }
    
    if (avformat_open_input(&pFormatContext, video_info.file_path.c_str(), NULL, NULL) != 0)
    {
        std::cerr << "ERROR could not open the file: " << video_info.file_path << '\n';
        return mat;
    }
    
    if (avformat_find_stream_info(pFormatContext,  NULL) < 0)
    {
        std::cerr << "ERROR could not get the stream info" << '\n';
        return mat;
    }
    
    AVCodec *pCodec = nullptr;
    AVCodecParameters *pCodecParameters =  nullptr;
    int video_stream_index = -1;
    
    ///////////////////////////////////////////////////////////////////
    
    for (int i = 0; i < pFormatContext->nb_streams; i++)
    {
        AVCodecParameters *pLocalCodecParameters =  NULL;
        pLocalCodecParameters = pFormatContext->streams[i]->codecpar;
        
        AVCodec *pLocalCodec = nullptr;
        
        pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);
        
        if(pLocalCodec == nullptr)
        {
            std::cerr << "ERROR unsupported codec!" << '\n';
            continue;
        }
        
        // when the stream is a video we store its index, codec parameters and codec
        if(pLocalCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            if (video_stream_index == -1)
            {
                video_stream_index = i;
                pCodec = pLocalCodec;
                pCodecParameters = pLocalCodecParameters;
                break;
            }
        }
    }

    AVCodecContext *pCodecContext = avcodec_alloc_context3(pCodec);
    if (!pCodecContext)
    {
        std::cerr << "ERROR failed to allocated memory for AVCodecContext" << '\n';
        return mat;
    }
    
    if (avcodec_parameters_to_context(pCodecContext, pCodecParameters) < 0)
    {
        std::cerr << "ERROR failed to copy codec params to codec context" << '\n';
        return mat;
    }

    if (avcodec_open2(pCodecContext, pCodec, nullptr) < 0)
    {
        std::cerr << "ERROR failed to open codec through avcodec_open2" << '\n';
        return mat;
    }

    AVFrame *pFrame = av_frame_alloc();
    if (!pFrame)
    {
        std::cerr << "ERROR failed to allocated memory for AVFrame" << '\n';
        return mat;
    }
    
    ////////////////////////////////////////////////
    
    // Allocate a color AVFrame structure
    AVFrame *pFrameRGB = av_frame_alloc();
    if(!pFrameRGB)
    {
        std::cerr << "ERROR failed to allocated memory for RGB AVFrame" << '\n';
        return mat;
    }

    // Determine required buffer size and allocate buffer
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, width, height, 1);
    uint8_t * buffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
    av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, buffer, AV_PIX_FMT_RGB24, width, height, 1);
    
    // initialize SWS context for software scaling
    struct SwsContext *sws_ctx = sws_getContext(pCodecContext->width,
                 pCodecContext->height,
                 pCodecContext->pix_fmt,
                 width,
                 height,
                 AV_PIX_FMT_RGB24,
                 SWS_BILINEAR,
                 NULL,
                 NULL,
                 NULL
                 );
    
    /////////////////////////////////////////////////

    AVPacket *pPacket = av_packet_alloc();
    if (!pPacket)
    {
        std::cerr << "ERROR failed to allocated memory for AVPacket" << '\n';
        return mat;
    }
    
    int response = 0;

    while (av_read_frame(pFormatContext, pPacket) >= 0)
    {
      // if it's the video stream
      if (pPacket->stream_index == video_stream_index)
      {
        response = decode_packet(pPacket, pCodecContext, pFrame);
        if(response == 0)
        {
            sws_scale(sws_ctx, (uint8_t const * const *)pFrame->data, pFrame->linesize, 0, pCodecContext->height, pFrameRGB->data, pFrameRGB->linesize);
            mat = cv::Mat(height, width, CV_8UC3, pFrameRGB->data[0]);
            cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR);
            av_packet_unref(pPacket);
            break;
        }
      }
      av_packet_unref(pPacket);
    }
    
    avformat_close_input(&pFormatContext);
    avformat_free_context(pFormatContext);
    av_packet_free(&pPacket);
    av_frame_free(&pFrame);
    av_frame_free(&pFrameRGB);
    avcodec_free_context(&pCodecContext);
    
    ///////////////////////////////////////////////////////////////////
    
    av_log_set_level(curr_log_level);
    
    return mat;
}


int VideoInfo::decode_packet(AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame)
{
    int response = avcodec_send_packet(pCodecContext, pPacket);
    
    if (response < 0)
    {
        //std::cerr << "Error while sending a packet to the decoder: " << av_err2str(response) << '\n';
        return response;
    }
    
    response = avcodec_receive_frame(pCodecContext, pFrame);
    if (response == AVERROR(EAGAIN))
    {
        //std::cerr << "Error while receiving a frame from the decoder: " << av_err2str(response) << '\n';
        return response;
    }
    if (response == AVERROR_EOF)
    {
        //std::cerr << "Error while receiving a frame from the decoder: " << av_err2str(response) << '\n';
        return response;
    }
    
    if (response < 0)
    {
        //std::cerr << "Error while receiving a frame from the decoder: " << av_err2str(response) << '\n';
        return response;
    }
    
    if (response >= 0)
    {
        //save_gray_frame(pFrame->data[0], pFrame->linesize[0], pFrame->width, pFrame->height, frame_filename);
    }
    
    return 0;
}

void VideoInfo::save_video_thumbnail_ppm(const VideoInfo &video_info, const std::string &thumbnail_path, int width, int height)
{
    std::cout << "Saving thumbnail: " << thumbnail_path << '\n';
    
    int curr_log_level = av_log_get_level();
    av_log_set_level(AV_LOG_QUIET);
    
    AVFormatContext *pFormatContext = avformat_alloc_context();
    if (!pFormatContext)
    {
        std::cerr << "ERROR could not allocate memory for Format Context" << '\n';
        return;
    }
    
    if (avformat_open_input(&pFormatContext, video_info.file_path.c_str(), NULL, NULL) != 0)
    {
        std::cerr << "ERROR could not open the file: " << video_info.file_path << '\n';
        return;
    }
    
    if (avformat_find_stream_info(pFormatContext,  NULL) < 0)
    {
        std::cerr << "ERROR could not get the stream info" << '\n';
        return;
    }
    
    AVCodec *pCodec = nullptr;
    AVCodecParameters *pCodecParameters =  nullptr;
    int video_stream_index = -1;
    
    ///////////////////////////////////////////////////////////////////
    
    for (int i = 0; i < pFormatContext->nb_streams; i++)
    {
        AVCodecParameters *pLocalCodecParameters =  NULL;
        pLocalCodecParameters = pFormatContext->streams[i]->codecpar;
        
        AVCodec *pLocalCodec = nullptr;
        
        pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);
        
        if(pLocalCodec == nullptr)
        {
            std::cerr << "ERROR unsupported codec!" << '\n';
            continue;
        }
        
        // when the stream is a video we store its index, codec parameters and codec
        if(pLocalCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            if (video_stream_index == -1)
            {
                video_stream_index = i;
                pCodec = pLocalCodec;
                pCodecParameters = pLocalCodecParameters;
                break;
            }
        }
    }

    AVCodecContext *pCodecContext = avcodec_alloc_context3(pCodec);
    if (!pCodecContext)
    {
        std::cerr << "ERROR failed to allocated memory for AVCodecContext" << '\n';
        return;
    }
    
    if (avcodec_parameters_to_context(pCodecContext, pCodecParameters) < 0)
    {
        std::cerr << "ERROR failed to copy codec params to codec context" << '\n';
        return;
    }

    if (avcodec_open2(pCodecContext, pCodec, nullptr) < 0)
    {
        std::cerr << "ERROR failed to open codec through avcodec_open2" << '\n';
        return;
    }

    AVFrame *pFrame = av_frame_alloc();
    if (!pFrame)
    {
        std::cerr << "ERROR failed to allocated memory for AVFrame" << '\n';
        return;
    }
    
    ////////////////////////////////////////////////
    
    // Allocate a color AVFrame structure
    AVFrame *pFrameRGB = av_frame_alloc();
    if(!pFrameRGB)
     {
          std::cerr << "ERROR failed to allocated memory for RGB AVFrame" << '\n';
          return;
      }

    // Determine required buffer size and allocate buffer
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, width, height, 1);
    uint8_t * buffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
    av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, buffer, AV_PIX_FMT_RGB24, width, height, 1);
    
    // initialize SWS context for software scaling
    struct SwsContext *sws_ctx = sws_getContext(pCodecContext->width,
                 pCodecContext->height,
                 pCodecContext->pix_fmt,
                 width,
                 height,
                 AV_PIX_FMT_RGB24,
                 SWS_BILINEAR,
                 NULL,
                 NULL,
                 NULL
                 );
    
    /////////////////////////////////////////////////

    AVPacket *pPacket = av_packet_alloc();
    if (!pPacket)
    {
        std::cerr << "ERROR failed to allocated memory for AVPacket" << '\n';
        return;
    }
    
    int response = 0;

    while (av_read_frame(pFormatContext, pPacket) >= 0)
    {
      // if it's the video stream
      if (pPacket->stream_index == video_stream_index)
      {
        response = decode_packet(pPacket, pCodecContext, pFrame);
        if(response == 0)
        {
            save_gray_frame(pFrame->data[0], pFrame->linesize[0], pFrame->width, pFrame->height, thumbnail_path);
            
            sws_scale(sws_ctx, (uint8_t const * const *)pFrame->data, pFrame->linesize, 0, pCodecContext->height, pFrameRGB->data, pFrameRGB->linesize);
            save_color_frame(pFrameRGB, width, height, thumbnail_path);
            
            av_packet_unref(pPacket);
            break;
        }
      }
      av_packet_unref(pPacket);
    }
    
    avformat_close_input(&pFormatContext);
    avformat_free_context(pFormatContext);
    av_packet_free(&pPacket);
    av_frame_free(&pFrame);
    av_frame_free(&pFrameRGB);
    avcodec_free_context(&pCodecContext);
    
    ///////////////////////////////////////////////////////////////////
    
    av_log_set_level(curr_log_level);
}


VideoInfo VideoInfo::get_video_info(const boost::filesystem::path &video_path)
{
    VideoInfo info;
    
    info.file_path = video_path.string();
    info.file_name = video_path.stem().string();
    info.file_ext = video_path.extension().string();
    
    int curr_log_level = av_log_get_level();
    av_log_set_level(AV_LOG_QUIET);
    
    // AVFormatContext holds the header information from the format (Container)
    // Allocating memory for this component
    // http://ffmpeg.org/doxygen/trunk/structAVFormatContext.html
    AVFormatContext *pFormatContext = avformat_alloc_context();
    if (!pFormatContext)
    {
        std::cerr << "ERROR could not allocate memory for Format Context" << '\n';
        return info;
    }
    
    // Open the file and read its header. The codecs are not opened.
    // The function arguments are:
    // AVFormatContext (the component we allocated memory for),
    // url (filename),
    // AVInputFormat (if you pass NULL it'll do the auto detect)
    // and AVDictionary (which are options to the demuxer)
    // http://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#ga31d601155e9035d5b0e7efedc894ee49
    if (avformat_open_input(&pFormatContext, info.file_path.c_str(), NULL, NULL) != 0)
    {
        std::cerr << "ERROR could not open the file: " << info.file_path << '\n';
        return info;
    }
    
    // now we have access to some information about our file
    // since we read its header we can say what format (container) it's
    // and some other information related to the format itself.
    info.iformat_name = pFormatContext->iformat->name;
    info.duration = pFormatContext->duration;
    info.bit_rate = pFormatContext->bit_rate;
    
    
    // read Packets from the Format to get stream information
    // this function populates pFormatContext->streams
    // (of size equals to pFormatContext->nb_streams)
    // the arguments are:
    // the AVFormatContext
    // and options contains options for codec corresponding to i-th stream.
    // On return each dictionary will be filled with options that were not found.
    // https://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#gad42172e27cddafb81096939783b157bb
    if (avformat_find_stream_info(pFormatContext,  NULL) < 0)
    {
        std::cerr << "ERROR could not get the stream info" << '\n';
        return info;
    }
    
    // the component that knows how to enCOde and DECode the stream
    // it's the codec (audio or video)
    // http://ffmpeg.org/doxygen/trunk/structAVCodec.html
    //AVCodec *pCodec = NULL;
    // this component describes the properties of a codec used by the stream i
    // https://ffmpeg.org/doxygen/trunk/structAVCodecParameters.html
    AVCodecParameters *pCodecParameters =  NULL;
    int video_stream_index = -1;
    
    
    // loop though all the streams and print its main information
    for (int i = 0; i < pFormatContext->nb_streams; i++)
    {
        AVCodecParameters *pLocalCodecParameters =  NULL;
        pLocalCodecParameters = pFormatContext->streams[i]->codecpar;
        
        StreamInfo sinfo;
        sinfo.codec_type = pLocalCodecParameters->codec_type;
        sinfo.time_base_num = pFormatContext->streams[i]->time_base.num;
        sinfo.time_base_den = pFormatContext->streams[i]->time_base.den;
        sinfo.frame_rate_num = pFormatContext->streams[i]->r_frame_rate.num;
        sinfo.frame_rate_den = pFormatContext->streams[i]->r_frame_rate.den;
        sinfo.start_time = pFormatContext->streams[i]->start_time;
        sinfo.duration = pFormatContext->streams[i]->duration;
        
        if(sinfo.codec_type == AVMEDIA_TYPE_DATA)
        {
            continue;
        }
        
        AVCodec *pLocalCodec = NULL;
        
        // finds the registered decoder for a codec ID
        // https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga19a0ca553277f019dd5b0fec6e1f9dca
        pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);
        
        if (pLocalCodec == NULL)
        {
            std::cerr << "ERROR unsupported codec, stream index " << i << ": " << avcodec_get_name(pLocalCodecParameters->codec_id) << '\n';
            info.streams.push_back(sinfo);
            continue;
        }
        
        if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            if (video_stream_index == -1)
            {
                video_stream_index = i;
                //pCodec = pLocalCodec;
                pCodecParameters = pLocalCodecParameters;
            }
            sinfo.width = pLocalCodecParameters->width;
            sinfo.height = pLocalCodecParameters->height;
        }
        else if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            sinfo.channels = pLocalCodecParameters->channels;
            sinfo.sample_rate = pLocalCodecParameters->sample_rate;
        }
        
        sinfo.codec_name = pLocalCodec->name;
        sinfo.codec_id = pLocalCodec->id;
        sinfo.bit_rate = pLocalCodecParameters->bit_rate;
        
        info.streams.push_back(sinfo);
    }
    
    avformat_close_input(&pFormatContext);
    avformat_free_context(pFormatContext);

    av_log_set_level(curr_log_level);
    
    return info;
}

std::vector<path> VideoInfo::get_video_paths(const std::string &folder, bool recurse)
{
    std::vector<boost::filesystem::path> paths;
    std::regex re("\\.(mp4|avi|mov)", std::regex::icase);
    path p(folder);
    for (auto i = directory_iterator(p); i != directory_iterator(); i++)
    {
        if (!is_directory(i->path()))
        {
            auto path = i->path();
            std::string video_filename = path.filename().string();
            
            if (std::regex_match(path.extension().string(), re))
            {
                paths.push_back(path);
            }
        }
        else if (recurse)
        {
            auto p = get_video_paths(i->path().string(), recurse);
            if(!p.empty())
            {
                paths.insert(std::end(paths), std::begin(p), std::end(p));
            }
        }
    }
    return paths;
}


void VideoInfo::save_gray_frame(unsigned char *buf, int wrap, int xsize, int ysize, const std::string& frame_filename)
{
    FILE *pFile = fopen(frame_filename.c_str(), "w");
    if(pFile == nullptr) return;
    fprintf(pFile, "P5\n%d %d\n%d\n", xsize, ysize, 255);
    for (int i = 0; i < ysize; i++)
        fwrite(buf + i * wrap, 1, xsize, pFile);
    fclose(pFile);
}

void VideoInfo::save_color_frame(AVFrame *pRGBFrame, int width, int height, const std::string& frame_filename)
{
    FILE *pFile = fopen(frame_filename.c_str(), "wb");
    if(pFile == nullptr) return;
    fprintf(pFile, "P6\n%d %d\n255\n", width, height);
    for(int y = 0; y < height; y++)
      fwrite(pRGBFrame->data[0]+y*pRGBFrame->linesize[0], 1, width*3, pFile);
    fclose(pFile);
}

void VideoInfo::output_txt_video_report(const std::vector<VideoInfo> &videoinfos, std::ostream &out)
{
    out << "Videos found: " << videoinfos.size() << '\n';
    out << '\n';

    for(auto &info : videoinfos)
    {
        out << "Path: " << info.file_path << '\n';
        out << "Video: " << info.file_name << '\n';
        out << "Extension: " << info.file_ext << '\n';
        out << "Format: " << info.iformat_name << '\n';
        out << "Duration (sec): " << info.duration / 100000 << '\n';
        out << "Bit Rate: " << info.bit_rate << '\n';
        out << "Number of streams: " << info.streams.size() << '\n';
        for(int i = 0; i < info.streams.size(); ++i)
        {
            auto &stream = info.streams[i];
            out << "Stream " << i << ":" << '\n';
            if(stream.codec_type == AVMEDIA_TYPE_VIDEO)
            {
                out << "Stream Type: " << "VIDEO" << '\n';
                out << "Resolution: " << stream.width << "x" << stream.height << '\n';
                out << "Codec name: " << stream.codec_name << '\n';
                out << "Avg Bitrate: " << stream.bit_rate / 1e6 << " Mb/s" << '\n';
            }
            else if(stream.codec_type == AVMEDIA_TYPE_AUDIO)
            {
                out << "Stream Type: " << "AUDIO" << '\n';
                out << "Sample Rate: " << stream.sample_rate << '\n';
                out << "Codec name: " << stream.codec_name << '\n';
            }
            else if(stream.codec_type == AVMEDIA_TYPE_DATA)
            {
                out << "Stream Type: " << "DATA" << '\n';
            }
            else if(stream.codec_type == AVMEDIA_TYPE_SUBTITLE)
            {
                out << "Stream Type: " << "SUBTITLE" << '\n';
            }
            else if(stream.codec_type == AVMEDIA_TYPE_ATTACHMENT)
            {
                out << "Stream Type: " << "ATTACHMENT" << '\n';
            }
            else if(stream.codec_type == AVMEDIA_TYPE_NB)
            {
                out << "Stream Type: " << "NB" << '\n';
            }
            else if(stream.codec_type == AVMEDIA_TYPE_UNKNOWN)
            {
                out << "Stream Type: " << "UNKNOWN" << '\n';
            }
            else
            {
                out << "Stream Type: " << "ERROR: NOT DETERMINED" << '\n';
            }
            out << "Start time: " << stream.start_time << '\n';
            out << "Duration: " << stream.duration << '\n';
            
        }
        std::cout << '\n';
    }
    std::cout << '\n';
}

void VideoInfo::output_html_video_report(const std::vector<VideoInfo> &videoinfos, std::ostream &out)
{
    out << "<html>" << '\n';
    
    out << "<!DOCTYPE html>" << '\n';
    out << "<html>" << '\n';
    out << "<head>" << '\n';
    out << "<style>" << '\n';
    out << "table {" << '\n';
    out << "  font-family: arial, sans-serif;" << '\n';
    out << "  border-collapse: collapse;" << '\n';
    out << "}" << '\n';

    out << "td, th {" << '\n';
    out << "  border: 1px solid #dddddd;" << '\n';
    out << "  text-align: left;" << '\n';
    out << "  padding: 8px;" << '\n';
    out << "}" << '\n';

    out << "tr:nth-child(even) {" << '\n';
    out << "  background-color: #dddddd;" << '\n';
    out << "}" << '\n';
    out << "</style>" << '\n';
    out << "</head>" << '\n';
    out << "<body>" << '\n';
    
    out << "<p/>" << '\n';
    out << "Videos found: " << videoinfos.size() << '\n';
    out << "<p/>" << '\n';
    out << '\n';

    for(auto &info : videoinfos)
    {
        out << "<table>" << '\n';
        
        std::string thubnail_name = info.file_name + ".jpg";
        int width = 320;
        int height = 180;
        
        save_video_thumbnail(info, thubnail_name, width, height);
        
        out << "<tr>" << '\n';
        out << "<td>" << info.file_name <<  "<img src=\"" << thubnail_name << "\" width=\"" << width << "\" height=\"" << height << "\" >" << "</td>" << '\n';
        out << "<td>";
        out <<      "<video width=\"" << width << "\" height=\"" << height << "\" controls>";
        out <<           "<source src=\"" << info.file_path << "\" type=\"video/mp4\">";
        out <<      "</video>";
        out << "</td>" << '\n';
        out << "</tr>" << '\n';
        
        out << "<tr><td>" << "Path: " << "</td><td>" <<  info.file_path << "</td></tr>" << '\n';
        out << "<tr><td>" << "Extension: " << "</td><td>" <<  info.file_ext << "</td></tr>" << '\n';
        out << "<tr><td>" << "Format: " << "</td><td>" <<  info.iformat_name << "</td></tr>" << '\n';
        out << "<tr><td>" << "Duration (sec): " << "</td><td>" <<  info.duration / 100000 << "</td></tr>" << '\n';
        out << "<tr><td>" << "Bit Rate: " << "</td><td>" <<  info.bit_rate << "</td></tr>" << '\n';
        out << "<tr><td>" << "Number of streams: " << "</td><td>" <<  info.streams.size() << "</td></tr>" << '\n';
        for(int i = 0; i < info.streams.size(); ++i)
        {
            auto &stream = info.streams[i];
            out << "<tr><td>" << "Stream " << i << ":" << "</td></tr>" << '\n';
            if(stream.codec_type == AVMEDIA_TYPE_VIDEO)
            {
                out << "<tr><td>" << "Stream Type: " << "</td><td>" <<  "VIDEO" << "</td></tr>" << '\n';
                out << "<tr><td>" << "Resolution: " << "</td><td>" <<  stream.width << "x" << stream.height << "</td></tr>" << '\n';
                out << "<tr><td>" << "Codec name: " << "</td><td>" <<  stream.codec_name << "</td></tr>" << '\n';
                out << "<tr><td>" << "Avg Bitrate: " << "</td><td>" <<  stream.bit_rate / 1e6 << " Mb/s" << "</td></tr>" << '\n';
            }
            else if(stream.codec_type == AVMEDIA_TYPE_AUDIO)
            {
                out << "<tr><td>" << "Stream Type: " << "</td><td>" <<  "AUDIO" << "</td></tr>" << '\n';
                out << "<tr><td>" << "Sample Rate: " << "</td><td>" <<  stream.sample_rate << "</td></tr>" << '\n';
                out << "<tr><td>" << "Codec name: " << "</td><td>" <<  stream.codec_name << "</td></tr>" << '\n';
            }
            else if(stream.codec_type == AVMEDIA_TYPE_DATA)
            {
                out << "<tr><td>" << "Stream Type: " << "</td><td>" <<  "DATA" << "</td></tr>" << '\n';
            }
            else if(stream.codec_type == AVMEDIA_TYPE_SUBTITLE)
            {
                out << "<tr><td>" << "Stream Type: " << "</td><td>" <<  "SUBTITLE" << "</td></tr>" << '\n';
            }
            else if(stream.codec_type == AVMEDIA_TYPE_ATTACHMENT)
            {
                out << "<tr><td>" << "Stream Type: " << "</td><td>" <<  "ATTACHMENT" << "</td></tr>" << '\n';
            }
            else if(stream.codec_type == AVMEDIA_TYPE_NB)
            {
                out << "<tr><td>" << "Stream Type: " << "</td><td>" <<  "NB" << "</td></tr>" << '\n';
            }
            else if(stream.codec_type == AVMEDIA_TYPE_UNKNOWN)
            {
                out << "<tr><td>" << "Stream Type: " << "</td><td>" <<  "UNKNOWN" << "</td></tr>" << '\n';
            }
            else
            {
                out << "<tr><td>" << "Stream Type: " << "</td><td>" <<  "ERROR: NOT DETERMINED" << "</td></tr>" << '\n';
            }
            out << "<tr><td>" << "Start time: " << "</td><td>" <<  stream.start_time << "</td></tr>" << '\n';
            out << "<tr><td>" << "Duration: " << "</td><td>" <<  stream.duration << "</td></tr>" << '\n';
            
        }
        out << "</table>" << '\n';
        out << "<br/>" << '\n';
        out << "<br/>" << '\n';
        out << "<br/>" << '\n';
        out << '\n';
    }
    out << '\n';
    out << "</body>" << '\n';
    out << "</html>" << '\n';
}
