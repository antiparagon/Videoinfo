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

#include "boost/program_options.hpp"

#include "VideoInfo.h"

using namespace videoinfo;

/*
 TODO:
 - Grab a frame from anywhere in a video
 - Add number of people count, chip-outs of peoples faces in html report
 */

int main(int argc, char** argv)
{
    std::string video_folder = ".";
    
    namespace po = boost::program_options;
    po::options_description opts_desc("Options");
    opts_desc.add_options()
        ("help,h", "Print help messages")
        ("dir,d", po::value<std::string>(), "Directory to scan");
    po::positional_options_description pos_opts_desc;
    pos_opts_desc.add("dir", 1);
    
    po::variables_map vm;
    try
    {
        po::store(po::command_line_parser(argc, argv).options(opts_desc)
          .positional(pos_opts_desc).run(),vm);

        if(vm.count("help") )
        {
          std::cout << "Videoinfo" << std::endl
                     << opts_desc << std::endl;
          return 0;
        }
        
        po::notify(vm); // throws on error, so do after help in case
        // there are any problems
        
        if(vm.count("dir"))
        {
          video_folder = vm["dir"].as<std::string>();
        }

    }
    catch(po::error& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
        std::cerr << opts_desc << std::endl;
        return 1;
    }
    
    auto paths = VideoInfo::get_video_paths(video_folder, true);
    
    std::vector<VideoInfo> video_infos;
    std::for_each(std::begin(paths), std::end(paths), [&video_infos](const boost::filesystem::path &video_path){
        std::cerr << "Opening: " << video_path.string() << '\n';
        video_infos.push_back(VideoInfo::get_video_info(video_path));
    });
    
//    std::for_each(std::begin(video_infos), std::end(video_infos), [](const VideoInfo& info){
//
//        int frame_num = 300;
//        cv::Mat mat = VideoInfo::get_frame_mat(info, frame_num, 640, 360);
//
//        if (!mat.empty())
//        {
//            cv::imwrite(info.file_name + "_" + std::to_string(frame_num) + ".jpg", mat);
//        }
//        else
//        {
//            std::cerr << "Couldn't create Mat for frame: " << info.file_path << '\n';
//            return;
//        }
//
//    });
    
    std::ofstream out("videos.html");
    VideoInfo::output_html_video_report(video_infos, out);
    
    return 0;
}






