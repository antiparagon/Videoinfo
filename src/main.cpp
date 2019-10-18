//
//  main.cpp
//  VideoInfo
//
//  Created by Antiparagon on 06/29/19.
//  Copyright © 2019 Antiparagon. All rights reserved.
//

#include <string>
#include <iostream>
#include <vector>

#include "VideoInfo.h"

using namespace videoinfo;

/*
 TODO:
 - Grab a frame from anywhere in a video
 - Update output_html_video_report to use JPEG images, link to videos
 - Add number of people count, chip-outs of peoples faces in html report
 */

int main()
{
    std::string video_folder = "/users/wmckay/Movies/Video/2019/2019-01-01/RX100";
    //std::string video_folder = "/users/wmckay/Movies/Video/2019/2019-01-01/A7Riii";
    //std::string video_folder = "/users/wmckay/Movies/Video/2019/2019-08-05";  // 37 videos
    //std::string video_folder = "/users/wmckay/Movies/Video/2019";
    //std::string video_folder = "/users/wmckay/Movies/Premiere Library 6";
    auto paths = VideoInfo::get_video_paths(video_folder, true);
    
    std::vector<VideoInfo> video_infos;
    std::for_each(std::begin(paths), std::end(paths), [&video_infos](const boost::filesystem::path &video_path){
        std::cerr << "Opening: " << video_path.string() << '\n';
        video_infos.push_back(VideoInfo::get_video_info(video_path));
    });
    
    /*
    VideoInfo::output_txt_video_report(video_infos, std::cout);
    return 0;
    //*/
    
    /*
    std::for_each(std::begin(video_infos), std::end(video_infos), [](const VideoInfo& video_info){
        std::string thumbnail_name = video_info.file_name + ".pgm";
        //VideoInfo::save_video_thumbnail(video_info, thumbnail_name, 320, 180);
        VideoInfo::save_video_thumbnail(video_info, thumbnail_name, 1920, 1080);
    });
    //*/
    
    /*
    std::all_of(std::begin(video_infos), std::end(video_infos), [](const VideoInfo& info){

        cv::Mat mat = VideoInfo::get_first_frame_mat(info, 1280, 720);
        
        if (mat.empty())
        {
            std::cout << "Could create Mat for first frame: " << info.file_path << '\n';
            return false;
        }
        
        std::string windowName = info.file_name;
        cv::namedWindow(windowName);
        cv::imshow(windowName, mat);
        int key = cv::waitKey(0);
        cv::destroyWindow(windowName);
        
        if(key == 's' || key == 'S') cv::imwrite(windowName + ".jpg", mat);
         
        return (key != 27);
    });
    //*/
    
    std::ofstream out("videoinfo.html");
    if(out)
    {
        VideoInfo::output_html_video_report(video_infos, out);
    }
    else
    {
        std::cerr << "Unable to open file for html output" << '\n';
    }
    return 0;
}






