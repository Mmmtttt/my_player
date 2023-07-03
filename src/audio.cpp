#include "audio.h"

    int                 i = 0;
    int                 a_idx = -1;
    int                 ret = 0;
    int                 res = 0;
SDL_AudioSpec       wanted_spec;

uint8_t *s_resample_buf = NULL;  // 重采样输出缓冲区
int s_resample_buf_len = 0;      // 重采样输出缓冲区长度

bool s_input_finished = false;   // 文件读取完毕
bool s_decode_finished = false;  // 解码完毕

// 新增全局变量以处理播放速率和播放位置
//static int s_audio_play_time = 0;         // 当前音频播放时间（毫秒）（解了当前包之后，应该处于的时间）
float s_audio_playback_rate = 1.0; // 音频播放速率

 FF_AudioParams s_audio_param_src;
 FF_AudioParams s_audio_param_tgt;


struct SwrContext *s_audio_swr_ctx;

int audio_decode_frame(AVCodecContext *p_codec_ctx, std::shared_ptr<myAVPacket> p_packet_ptr, uint8_t *audio_buf, int buf_size)
{
    if(s_playing_exit)exit(0);

    AVFrame *p_frame = av_frame_alloc();
    
    int frm_size = 0;
    int res = 0;
    int ret = 0;
    int nb_samples = 0;             // 重采样输出样本数
    uint8_t *p_cp_buf = NULL;
    int cp_len = 0;
    bool need_new = false;
    res = 0;
    while (1)
    {
        need_new = false;
        
        // 1 接收解码器输出的数据，每次接收一个frame
        ret = avcodec_receive_frame(p_codec_ctx, p_frame);
        if (ret != 0)
        {
            if (ret == AVERROR_EOF)
            {
                printf("audio avcodec_receive_frame(): the decoder has been fully flushed\n");
                res = 0;
                goto exit;
            }
            else if (ret == AVERROR(EAGAIN))
            {
                //printf("audio avcodec_receive_frame(): output is not available in this state - "
                //       "user must try to send new input\n");
                need_new = true;
            }
            else if (ret == AVERROR(EINVAL))
            {
                printf("audio avcodec_receive_frame(): codec not opened, or it is an encoder\n");
                res = -1;
                goto exit;
            }
            else
            {
                printf("audio avcodec_receive_frame(): legitimate decoding errors\n");
                res = -1;
                goto exit;
            }
        }
        else
        {
            // s_audio_param_tgt是SDL可接受的音频帧数，是main()中取得的参数
            // 在main()函数中又有“s_audio_param_src = s_audio_param_tgt”
            // 此处表示：如果frame中的音频参数 == s_audio_param_src == s_audio_param_tgt，那音频重采样的过程就免了(因此时s_audio_swr_ctx是NULL)
            // 　　　　　否则使用frame(源)和s_audio_param_src(目标)中的音频参数来设置s_audio_swr_ctx，并使用frame中的音频参数来赋值s_audio_param_src
            if (p_frame->format         != s_audio_param_src.fmt            ||
                p_frame->channel_layout != s_audio_param_src.channel_layout ||
                p_frame->sample_rate    != s_audio_param_src.freq)
            {
                swr_free(&s_audio_swr_ctx);
                // 使用frame(源)和is->audio_tgt(目标)中的音频参数来设置is->swr_ctx
                s_audio_swr_ctx = swr_alloc_set_opts(NULL,
                                                     s_audio_param_tgt.channel_layout, 
                                                     s_audio_param_tgt.fmt, 
                                                     s_audio_param_tgt.freq,
                                                     p_frame->channel_layout,           
                                                     (AVSampleFormat)p_frame->format, 
                                                     p_frame->sample_rate,
                                                     0,
                                                     NULL);
                if (s_audio_swr_ctx == NULL || swr_init(s_audio_swr_ctx) < 0)
                {
                    printf("Cannot create sample rate converter for conversion of %d Hz %s %d channels to %d Hz %s %d channels!\n",
                            p_frame->sample_rate, av_get_sample_fmt_name((AVSampleFormat)p_frame->format), p_frame->channels,
                            s_audio_param_tgt.freq, av_get_sample_fmt_name(s_audio_param_tgt.fmt), s_audio_param_tgt.channels);
                    swr_free(&s_audio_swr_ctx);
                    return -1;
                }
                
                // 使用frame中的参数更新s_audio_param_src，第一次更新后后面基本不用执行此if分支了，因为一个音频流中各frame通用参数一样
                s_audio_param_src.channel_layout = p_frame->channel_layout;
                s_audio_param_src.channels       = p_frame->channels;
                s_audio_param_src.freq           = p_frame->sample_rate;
                s_audio_param_src.fmt            = (AVSampleFormat)p_frame->format;
            }

            if (s_audio_swr_ctx != NULL)        // 重采样
            {
                // 重采样输入参数1：输入音频样本数是p_frame->nb_samples
                // 重采样输入参数2：输入音频缓冲区
                const uint8_t **in = (const uint8_t **)p_frame->extended_data;
                // 重采样输出参数1：输出音频缓冲区尺寸
                // 重采样输出参数2：输出音频缓冲区
                uint8_t **out = &s_resample_buf;
                // 重采样输出参数：输出音频样本数(多加了256个样本)
                int out_count = (int64_t)p_frame->nb_samples * s_audio_param_tgt.freq / p_frame->sample_rate + 256;
                // 重采样输出参数：输出音频缓冲区尺寸(以字节为单位)
                int out_size  = av_samples_get_buffer_size(NULL, s_audio_param_tgt.channels, out_count, s_audio_param_tgt.fmt, 0);
                if (out_size < 0)
                {
                    printf("av_samples_get_buffer_size() failed\n");
                    return -1;
                }
                
                if (s_resample_buf == NULL)
                {
                    av_fast_malloc(&s_resample_buf, (unsigned int *)&s_resample_buf_len, out_size);
                }
                if (s_resample_buf == NULL)
                {
                    return AVERROR(ENOMEM);
                }
                // 音频重采样：返回值是重采样后得到的音频数据中单个声道的样本数
                nb_samples = swr_convert(s_audio_swr_ctx, out, out_count, in, p_frame->nb_samples);
                if (nb_samples < 0) {
                    printf("swr_convert() failed\n");
                    return -1;
                }
                if (nb_samples == out_count)
                {
                    printf("audio buffer is probably too small\n");
                    if (swr_init(s_audio_swr_ctx) < 0)
                        swr_free(&s_audio_swr_ctx);
                }
        
                // 重采样返回的一帧音频数据大小(以字节为单位)
                p_cp_buf = s_resample_buf;
                cp_len = nb_samples * s_audio_param_tgt.channels * av_get_bytes_per_sample(s_audio_param_tgt.fmt);
            }
            else    // 不重采样
            {
                // 根据相应音频参数，获得所需缓冲区大小
                frm_size = av_samples_get_buffer_size(
                        NULL, 
                        p_codec_ctx->channels,
                        p_frame->nb_samples,
                        p_codec_ctx->sample_fmt,
                        1);
                
                printf("frame size %d, buffer size %d\n", frm_size, buf_size);
                assert(frm_size <= buf_size);

                p_cp_buf = p_frame->data[0];
                cp_len = frm_size;
            }
            
            // 将音频帧拷贝到函数输出参数audio_buf
            memcpy(audio_buf, p_cp_buf, cp_len);
            res = cp_len;
            goto exit;
        }
        //std::cout<<"333"<<std::endl;

        // 2 向解码器喂数据，每次喂一个packet
        if (need_new)
        {
            //myAVPacket temp=p_packet;
            ret = avcodec_send_packet(p_codec_ctx, &(p_packet_ptr->mypkt));
            if (ret != 0)
            {
                printf("avcodec_send_packet() failed %d\n", ret);
                //av_packet_unref(p_packet);

                res = -1;
                goto exit;
            }
        }
    }

exit:
    av_frame_unref(p_frame);
    return res;
}


void sdl_audio_callback(void *userdata, uint8_t *stream, int len)
{
    if(s_playing_exit)exit(0);

    AVCodecContext *p_codec_ctx = (AVCodecContext *)userdata;
    int copy_len;           // 
    int get_size;           // 获取到解码后的音频数据大小

    static uint8_t s_audio_buf[(MAX_AUDIO_FRAME_SIZE*3)/2]; // 1.5倍声音帧的大小
    static uint32_t s_audio_len = 0;    // 新取得的音频数据大小
    static uint32_t s_tx_idx = 0;       // 已发送给设备的数据量

    std::shared_ptr<myAVPacket> p_packet_ptr;

    int frm_size = 0;
    int ret_size = 0;
    int ret;

    while (len > 0)         // 确保stream缓冲区填满，填满后此函数返回
    {
        if (s_decode_finished)
        {
            return;
        }

        if (s_tx_idx >= s_audio_len)
        {   // audio_buf缓冲区中数据已全部取出，则从队列中获取更多数据
            
            //std::cout<<p_packet.size<<std::endl;

            while (1)
            {
                // 1. 从队列中读出一包音频数据
                if (audio_packet_queue.packet_queue_pop(p_packet_ptr, 1) <= 0)
                {
                    if (s_input_finished)
                    {
                        // av_packet_unref(p_packet);
                        // p_packet = NULL;    // flush decoder
                        printf("Flushing decoder...\n");
                    }
                    else
                    {
                        printf("1\n");
                        // av_packet_unref(p_packet);
                        return;
                    }
                }

                


                // 检查包是否在播放时间之前，如果是，则将其跳过
                s_audio_play_time=p_packet_ptr->mypkt.pts * av_q2d(p_codec_ctx->time_base) * 1000.0;
                auto end = std::chrono::high_resolution_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

                time_shaft+=(elapsed.count()-a_last_time)*speed;//时间轴，是一次
                a_last_time=elapsed.count();

                std::cout<<"audio: "<<time_shaft<<" - "<<s_video_play_time<<" = "<<time_shaft-s_audio_play_time<<std::endl;
                int64_t diff=time_shaft-s_audio_play_time;
                if (50 <= diff)
                {
                    continue;
                }
                else if(-50>=diff)
                {
                    audio_packet_queue.curr_decode_pos=audio_packet_queue.curr_decode_pos-2;
                    continue;
                }
                else{break;}
            }
            // 解码并根据播放速率处理
            s_audio_len = audio_decode_frame(p_codec_ctx, p_packet_ptr, s_audio_buf, sizeof(s_audio_buf)) / s_audio_playback_rate;
            s_tx_idx = 0;
            
        }

        copy_len = s_audio_len - s_tx_idx;
        if (copy_len > len)
        {
            copy_len = len;
        }

        // 将解码后的音频帧(s_audio_buf+)写入音频设备缓冲区(stream)，播放
        memcpy(stream, (uint8_t *)s_audio_buf + s_tx_idx, copy_len);
        len -= copy_len;
        stream += copy_len;
        s_tx_idx += copy_len;

        
    }
    if (audio_packet_queue.size == 0 && s_input_finished)
    {
        s_decode_finished = true;
    }
}
// int64_t myAVPacket::num=0;

