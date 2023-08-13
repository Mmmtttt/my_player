#include "video.h"
#include <winsock2.h>
#include <ws2tcpip.h>

// std::chrono::_V2::system_clock::time_point start;
// int64_t time_shaft = 0;
// int64_t a_last_time = 0;
// int64_t v_last_time = 0;
// double speed = 1.0;
// bool s_playing_pause = false;
// bool s_playing_exit = false;
// int64_t s_audio_play_time = 0;
// int64_t s_video_play_time = 0;

// SOCKET listen_socket;
// sockaddr_in serverService;
// SOCKET accept_socket;
// SOCKET client_socket;
// sockaddr_in clientService;



int main(int argc, char* argv[])
{
    
    AVFormatContext *inputContext=NULL;
    
    AVStream *videoStream = NULL, *audioStream = NULL;
    AVPacket packet;

    avformat_open_input(&inputContext, "BAB-026.ts", NULL, NULL);
    avformat_find_stream_info(inputContext, NULL);

    AVCodecParameters *v_p_codec_par0 = inputContext->streams[0]->codecpar;
    AVCodecParameters *a_p_codec_par0 = inputContext->streams[1]->codecpar;
    const AVCodec* v_p_codec0 = avcodec_find_decoder(v_p_codec_par0->codec_id);
    const AVCodec* a_p_codec0 = avcodec_find_decoder(a_p_codec_par0->codec_id);
    AVCodecContext *v_p_codec_ctx0 = avcodec_alloc_context3(v_p_codec0);
    AVCodecContext *a_p_codec_ctx0 = avcodec_alloc_context3(a_p_codec0);
    avcodec_parameters_to_context(v_p_codec_ctx0, v_p_codec_par0);
    avcodec_parameters_to_context(a_p_codec_ctx0, a_p_codec_par0);
    avcodec_open2(v_p_codec_ctx0, v_p_codec0, NULL);
    avcodec_open2(a_p_codec_ctx0, a_p_codec0, NULL);

    // Find video and audio streams
    for (int i = 0; i < inputContext->nb_streams; i++) {
        if (inputContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = inputContext->streams[i];
        } else if (inputContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStream = inputContext->streams[i];
        }
    }
















    // Open output file context
    AVFormatContext *outputContext = NULL;
    avformat_alloc_output_context2(&outputContext, NULL, NULL, "BAB-026.mp4");



    // Copy video and audio streams to output file
    const AVCodec* v_p_codec = avcodec_find_encoder_by_name("libx264");
    AVStream *outVideoStream = avformat_new_stream(outputContext, v_p_codec);

    const AVCodec* a_p_codec = avcodec_find_encoder_by_name("aac");
    AVStream *outAudioStream = avformat_new_stream(outputContext, a_p_codec);
    
    
    
    
    avcodec_parameters_copy(outVideoStream->codecpar, videoStream->codecpar);
    avcodec_parameters_copy(outAudioStream->codecpar, audioStream->codecpar);



    // 获取输入视频流的宽度和高度
    int inWidth = videoStream->codecpar->width;
    int inHeight = videoStream->codecpar->height;

    // 设置输出视频流的宽度和高度
    outVideoStream->codecpar->width = inWidth;
    outVideoStream->codecpar->height = inHeight;
    outVideoStream->codecpar->codec_tag = av_codec_get_tag(outputContext->oformat->codec_tag, outVideoStream->codecpar->codec_id);
    outVideoStream->codecpar->format = AV_PIX_FMT_YUV420P;
    outAudioStream->codecpar->codec_tag = av_codec_get_tag(outputContext->oformat->codec_tag, outAudioStream->codecpar->codec_id); 


    AVCodecContext *v_p_codec_ctx = avcodec_alloc_context3(v_p_codec);
    AVCodecContext *a_p_codec_ctx = avcodec_alloc_context3(a_p_codec);
    avcodec_parameters_to_context(v_p_codec_ctx, videoStream->codecpar);
    avcodec_parameters_to_context(a_p_codec_ctx, audioStream->codecpar);
    avcodec_open2(v_p_codec_ctx, v_p_codec, NULL);
    avcodec_open2(a_p_codec_ctx, a_p_codec, NULL);

    // Open output file
    if (avio_open(&outputContext->pb, "BAB-026.mp4", AVIO_FLAG_WRITE) < 0) {
        fprintf(stderr, "Could not open output file\n");
        return -1;
    }

    // Write file header
    avformat_write_header(outputContext, NULL);

    // Set start and duration for trimming
    int64_t start_time = av_rescale_q(30, AV_TIME_BASE_Q, videoStream->time_base);
    int64_t duration = av_rescale_q(1000, AV_TIME_BASE_Q, videoStream->time_base);

    // Read and write packets
    while (av_read_frame(inputContext, &packet) >= 0) {
        // if (packet.pts < start_time || packet.pts > start_time + duration) {
        //     av_packet_unref(&packet);
        //     continue;
        // }

        AVStream *inStream = inputContext->streams[packet.stream_index];
        AVStream *outStream = outputContext->streams[packet.stream_index];

        // Write packet to output
        packet.pts = av_rescale_q_rnd(packet.pts, inStream->time_base, outStream->time_base, AV_ROUND_NEAR_INF);
        packet.dts = av_rescale_q_rnd(packet.dts, inStream->time_base, outStream->time_base, AV_ROUND_NEAR_INF);
        packet.duration = av_rescale_q(packet.duration, inStream->time_base, outStream->time_base);
        packet.pos = -1;

        // 解码数据包为原始帧
    if (packet.stream_index == 0) {
        avcodec_send_packet(v_p_codec_ctx0, &packet);
        AVFrame *decodedFrame = av_frame_alloc();
        avcodec_receive_frame(v_p_codec_ctx0, decodedFrame);

        // 格式转换（将视频帧转换为目标像素格式）
        AVPixelFormat targetPixelFormat = AV_PIX_FMT_YUV420P; // 替换为你的目标像素格式
        AVFrame *convertedFrame = av_frame_alloc();
        convertedFrame->width = outStream->codecpar->width;
        convertedFrame->height = outStream->codecpar->height;
        convertedFrame->format = targetPixelFormat;
        av_frame_get_buffer(convertedFrame, 0);
        
        struct SwsContext *swsContext = sws_getContext(
            v_p_codec_ctx0->width, v_p_codec_ctx0->height, v_p_codec_ctx0->pix_fmt,
            outStream->codecpar->width, outStream->codecpar->height, targetPixelFormat,
            SWS_BICUBIC, NULL, NULL, NULL);
        sws_scale(swsContext, decodedFrame->data, decodedFrame->linesize,
                  0, v_p_codec_ctx0->height, convertedFrame->data, convertedFrame->linesize);
        sws_freeContext(swsContext);
        
        // 编码转换后的帧为目标数据包
        avcodec_send_frame(v_p_codec_ctx, convertedFrame);
        av_packet_unref(&packet);
        AVPacket targetPacket;
        av_init_packet(&targetPacket);
        targetPacket.data = NULL;
        targetPacket.size = 0;
        avcodec_receive_packet(v_p_codec_ctx, &targetPacket);
        
        // 写入目标数据包
        av_interleaved_write_frame(outputContext, &targetPacket);
        
        av_frame_free(&decodedFrame);
        av_frame_free(&convertedFrame);
        av_packet_unref(&targetPacket);
    } else if (packet.stream_index == 1) {
        // 类似地处理音频数据包
        // 注意：音频数据包的解码和编码可能涉及到音频采样格式的转换等
    }
    }

    // Write file trailer
    av_write_trailer(outputContext);

    // Clean up
    avformat_close_input(&inputContext);
    avio_close(outputContext->pb);
    avformat_free_context(outputContext);
    
    std::cout<<sizeof(myAVPacket)<<std::endl;
    std::string filename;

    if (argc < 2) {
        std::cin>>filename;
    }
    else{filename=argv[1];}
    
    Video video(filename);
    
    video.push_All_Packets();

    video.play();
    video.~Video();
    SDL_Delay(5000);


    return 0;
}
