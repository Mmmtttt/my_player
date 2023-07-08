#include "video.h"
#include <iostream>
#include <libavformat/avformat.h>




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
    for (int i=0; i<p_fmt_ctx->nb_streams; i++)
    {
        if (p_fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            v_idx = i;
            printf("Find a video stream, index %d\n", v_idx);
            // frame_rate = p_fmt_ctx->streams[i]->avg_frame_rate.num /
            //              p_fmt_ctx->streams[i]->avg_frame_rate.den;
            
        }
        else if(p_fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            a_idx = i;
            printf("Find a audio stream, index %d\n", a_idx);
            
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

    v_p_codec_par = p_fmt_ctx->streams[v_idx]->codecpar;
    a_p_codec_par = p_fmt_ctx->streams[a_idx]->codecpar;

    v_timebase_in_ms =av_q2d(p_fmt_ctx->streams[v_idx]->time_base) * 1000;
    a_timebase_in_ms =av_q2d(p_fmt_ctx->streams[a_idx]->time_base) * 1000;



    try{v_decoder=std::make_unique<videoDecoder>(v_p_codec_par, v_idx ,v_timebase_in_ms);}
    catch(const std::exception& e)
    {
        std::cout<<e.what()<<std::endl;
        avformat_close_input(&p_fmt_ctx);
        throw std::runtime_error("create v_decoder failed\n");
    } 
    
    try{static_a_decoder=std::make_shared<audioDecoder>(a_p_codec_par, a_idx,a_timebase_in_ms);}
    catch(const std::exception& e)
    {
        std::cout<<e.what()<<std::endl;
        avformat_close_input(&p_fmt_ctx);
        throw std::runtime_error("create a_decoder failed\n");
    }
    a_decoder=static_a_decoder;

    video_packet_queue=std::make_shared<packetQueue>();
    audio_packet_queue=std::make_shared<packetQueue>();
    v_decoder->video_packet_queue=video_packet_queue;
    a_decoder->audio_packet_queue=audio_packet_queue;
    
//std::cout<<"done2"<<std::endl;
    
}

Video::~Video()
{
    avformat_close_input(&p_fmt_ctx);
    
    avformat_network_deinit();
}

Video::Video(const std::string& _filename,TYPE type):filename(_filename){
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
    for (int i=0; i<p_fmt_ctx->nb_streams; i++)
    {
        if (p_fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            v_idx = i;
            printf("Find a video stream, index %d\n", v_idx);
            // frame_rate = p_fmt_ctx->streams[i]->avg_frame_rate.num /
            //              p_fmt_ctx->streams[i]->avg_frame_rate.den;
            
        }
        else if(p_fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            a_idx = i;
            printf("Find a audio stream, index %d\n", a_idx);
            
        }
    }
    if (v_idx == -1)
    {
        avformat_close_input(&p_fmt_ctx);
        printf("Cann't find a video stream\n");
    }if (a_idx == -1)
    {
        avformat_close_input(&p_fmt_ctx);
        printf("Cann't find a audio stream\n");
    }

    v_p_codec_par = p_fmt_ctx->streams[v_idx]->codecpar;
    a_p_codec_par = p_fmt_ctx->streams[a_idx]->codecpar;

    v_timebase_in_ms =av_q2d(p_fmt_ctx->streams[v_idx]->time_base) * 1000;
    a_timebase_in_ms =av_q2d(p_fmt_ctx->streams[a_idx]->time_base) * 1000;

    video_packet_queue=std::make_shared<packetQueue>();
    audio_packet_queue=std::make_shared<packetQueue>();

}


Video::Video(int _v_idx,AVCodecParameters *_v_p_codec_par,double _v_timebase_in_ms,int _a_idx,AVCodecParameters *_a_p_codec_par,double _a_timebase_in_ms){
    v_p_codec_par = _v_p_codec_par;
    a_p_codec_par = _a_p_codec_par;

    v_timebase_in_ms =_v_timebase_in_ms;
    a_timebase_in_ms =_a_timebase_in_ms;

    v_idx=_v_idx;
    a_idx=_a_idx;




    try{v_decoder=std::make_unique<videoDecoder>(v_p_codec_par, v_idx ,v_timebase_in_ms);}
    catch(const std::exception& e)
    {
        std::cout<<e.what()<<std::endl;
        avformat_close_input(&p_fmt_ctx);
        throw std::runtime_error("create v_decoder failed\n");
    } 
    
    try{static_a_decoder=std::make_unique<audioDecoder>(a_p_codec_par, a_idx,a_timebase_in_ms);}
    catch(const std::exception& e)
    {
        std::cout<<e.what()<<std::endl;
        avformat_close_input(&p_fmt_ctx);
        throw std::runtime_error("create a_decoder failed\n");
    }
    a_decoder=static_a_decoder;
    filename=std::string("1.mp4");

    video_packet_queue=std::make_shared<packetQueue>();
    audio_packet_queue=std::make_shared<packetQueue>();
    v_decoder->video_packet_queue=video_packet_queue;
    a_decoder->audio_packet_queue=audio_packet_queue;
}



void Video::play(){
    // v_decoder->push_All_Packets(p_fmt_ctx);
    start = std::chrono::high_resolution_clock::now();
    SDL_PauseAudio(0);
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
                
                if(s_playing_pause)action();
                else pause();
            }
            else if (sdl_event.key.keysym.sym == SDLK_LEFT)
            {
                if(time_shaft>5000)time_shaft-=5000;
                else time_shaft=0;
                avcodec_flush_buffers(v_decoder->p_codec_ctx);
            }
            else if (sdl_event.key.keysym.sym == SDLK_RIGHT)
            {
                time_shaft+=5000;
                avcodec_flush_buffers(v_decoder->p_codec_ctx);
            }
            else if (sdl_event.key.keysym.sym == SDLK_UP)
            {
                speed+=0.25;
            }
            else if (sdl_event.key.keysym.sym == SDLK_DOWN)
            {
                if(speed>0.5)speed-=0.25;
            }
            else if (sdl_event.key.keysym.sym == SDLK_9)
            {
                time_shaft=20*60*1000;
            }
            
        }
        else if (sdl_event.type == SDL_QUIT)
        {
            SDL_PauseAudio(1);
            // 用户按下关闭窗口按钮
            printf("SDL event QUIT\n");
            s_playing_exit = true;
            SDL_Quit();
            break;
        }
        else
        {
            // printf("Ignore SDL event 0x%04X\n", sdl_event.type);
        }
    }
}

void Video::push_All_Packets(){
    int ret=0;
    int64_t num=0;
    while(ret==0){
        std::shared_ptr<myAVPacket> temp(new myAVPacket());
        ret=av_read_frame(p_fmt_ctx, &temp->mypkt);
        temp->size=temp->mypkt.size;
        
        num++;
        temp->num=num;
        if(temp->mypkt.stream_index==AVMEDIA_TYPE_VIDEO){
            temp->id_in_queue=video_packet_queue->pkts_ptr.size();
            video_packet_queue->packet_queue_push(temp);
            temp->is_recived=true;
        }
            
        else if(temp->mypkt.stream_index==AVMEDIA_TYPE_AUDIO){
            temp->id_in_queue=audio_packet_queue->pkts_ptr.size();
            audio_packet_queue->packet_queue_push(temp);
            temp->is_recived=true;
        }
    }
}