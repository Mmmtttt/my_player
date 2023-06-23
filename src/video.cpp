#include "video.h"
#include <iostream>
#include <libavformat/avformat.h>

bool s_playing_exit =false;
bool s_playing_pause =false;

Video::Video(const std::string& filename):filename(filename)
{
    avformat_network_init();
    //av_register_all();
    // A1. 打开视频文件：读取文件头，将文件格式信息存储在"fmt context"中
    int ret = avformat_open_input(&p_fmt_ctx, filename.c_str(), NULL, NULL);
    if (ret != 0)
    {
        throw std::runtime_error("avformat_open_input() failed ");
    }

    // A2. 搜索流信息：读取一段视频文件数据，尝试解码，将取到的流信息填入pFormatCtx->streams
    //     p_fmt_ctx->streams是一个指针数组，数组大小是pFormatCtx->nb_streams
    ret = avformat_find_stream_info(p_fmt_ctx, NULL);
    if (ret < 0)
    {
        avformat_close_input(&p_fmt_ctx);
        throw std::runtime_error("avformat_find_stream_info() failed ");
    }


    // A3. 查找第一个视频流
    v_idx = -1;
    for (i=0; i<p_fmt_ctx->nb_streams; i++)
    {
        if (p_fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            v_idx = i;
            printf("Find a video stream, index %d\n", v_idx);
            frame_rate = p_fmt_ctx->streams[i]->avg_frame_rate.num /
                         p_fmt_ctx->streams[i]->avg_frame_rate.den;
            break;
        }
        else if(p_fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            a_idx = i;
            printf("Find a audio stream, index %d\n", a_idx);
            break;
        }
    }
    if (v_idx == -1)
    {
        avformat_close_input(&p_fmt_ctx);
        throw std::runtime_error("Cann't find a video stream\n");
    }if (a_idx == -1)
    {
        avformat_close_input(&p_fmt_ctx);
        throw std::runtime_error("Cann't find a audio stream\n");
    }




    try{v_decoder=std::make_unique<Decoder>(p_fmt_ctx, v_idx ,frame_rate);}
    catch(const std::exception& e)
    {
        std::cout<<e.what()<<std::endl;
        avformat_close_input(&p_fmt_ctx);
        throw std::runtime_error("create v_decoder failed\n");
    } 
    try{a_decoder=std::make_unique<Decoder>(p_fmt_ctx, a_idx,frame_rate);}
    catch(const std::exception& e)
    {
        std::cout<<e.what()<<std::endl;
        avformat_close_input(&p_fmt_ctx);
        throw std::runtime_error("create a_decoder failed\n");
    }
    
    
    
}

Video::~Video()
{
    avformat_close_input(&p_fmt_ctx);
    
    avformat_network_deinit();
}




// void Video::read_One_frame_packet(){
//     // A8. 从视频文件中读取一个packet
//     //     packet可能是视频帧、音频帧或其他数据，解码器只会解码视频帧或音频帧，非音视频数据并不会被
//     //     扔掉、从而能向解码器提供尽可能多的信息
//     //     对于视频来说，一个packet只包含一个frame
//     //     对于音频来说，若是帧长固定的格式则一个packet可包含整数个frame，
//     //                   若是帧长可变的格式则一个packet只包含一个frame
//     while (av_read_frame(p_fmt_ctx, v_decoder->p_packet) == 0)
//     {
//         if (v_decoder->p_packet->stream_index == v_idx)  // 取到一帧视频帧，则退出
//         {
//             break;
//         }
//     }
// }

void Video::play(){
    v_decoder->get_Packets(p_fmt_ctx);
    a_decoder->get_Packets(p_fmt_ctx);
    while(1){
        // B6. 等待刷新事件
        SDL_WaitEvent(&sdl_event);
        if (sdl_event.type == SDL_USEREVENT_REFRESH)
        {
            //read_One_frame_packet();
            int ret;
            try{ret=v_decoder->present_One_frame();}
            catch(const std::exception&e){
                std::cout<<e.what()<<std::endl;
                return;
            } 
            if(ret==-1) continue;
        }
        else if (sdl_event.type == SDL_KEYDOWN)
        {
            if (sdl_event.key.keysym.sym == SDLK_SPACE)
            {
                // 用户按空格键，暂停/继续状态切换
                s_playing_pause = !s_playing_pause;
                printf("player %s\n", s_playing_pause ? "pause" : "continue");
            }
        }
        else if (sdl_event.type == SDL_QUIT)
        {
            // 用户按下关闭窗口按钮
            printf("SDL event QUIT\n");
            s_playing_exit = true;
            break;
        }
        else
        {
            // printf("Ignore SDL event 0x%04X\n", sdl_event.type);
        }
    }
}