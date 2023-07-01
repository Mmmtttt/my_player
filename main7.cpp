#include <iostream>
#include <cstdlib>
#include <cstring>
extern "C"{
#include <SDL.h>
#include <libavformat/avformat.h>
#include <libavutil/time.h>
}

#define PACKET_QUEUE_SIZE 100



AVFrame *frame;

// 音频包队列
typedef struct {
    AVPacket *packets[PACKET_QUEUE_SIZE];
    int size;
    int front;
    int rear;
    SDL_mutex *mutex;
    SDL_cond *cond;
} PacketQueue;

// 音频时钟
typedef struct {
    double pts;
    double speed;
    int paused;
    int64_t pausedPts;
    int64_t basePts;
    SDL_mutex *mutex;
} AudioClock;

PacketQueue audioQueue;  // 音频包队列
AudioClock audioClock;   // 音频时钟

int audioStreamIndex = -1;
AVFormatContext *formatContext = nullptr;
AVCodecContext *codecContext = nullptr;
SDL_AudioDeviceID audioDeviceID = 0;

// 初始化音频包队列
void initPacketQueue(PacketQueue *queue) {
    queue->size = 0;
    queue->front = 0;
    queue->rear = 0;
    queue->mutex = SDL_CreateMutex();
    queue->cond = SDL_CreateCond();
}

// 销毁音频包队列
void destroyPacketQueue(PacketQueue *queue) {
    SDL_DestroyMutex(queue->mutex);
    SDL_DestroyCond(queue->cond);
}

// 入队音频包
int enqueuePacket(PacketQueue *queue, AVPacket *packet) {
    SDL_LockMutex(queue->mutex);

    // 等待队列有空位
    while (queue->size >= PACKET_QUEUE_SIZE) {
        SDL_CondWait(queue->cond, queue->mutex);
    }

    // 入队
    queue->packets[queue->rear] = av_packet_clone(packet);
    queue->rear = (queue->rear + 1) % PACKET_QUEUE_SIZE;
    queue->size++;

    SDL_CondSignal(queue->cond);
    SDL_UnlockMutex(queue->mutex);

    return 0;
}

// 出队音频包
int dequeuePacket(PacketQueue *queue, AVPacket *packet) {
    SDL_LockMutex(queue->mutex);

    // 等待队列有数据
    while (queue->size <= 0) {
        SDL_CondWait(queue->cond, queue->mutex);
    }

    // 出队
    av_packet_move_ref(packet, queue->packets[queue->front]);
    av_packet_unref(queue->packets[queue->front]);
    queue->front = (queue->front + 1) % PACKET_QUEUE_SIZE;
    queue->size--;

    SDL_CondSignal(queue->cond);
    SDL_UnlockMutex(queue->mutex);

    return 0;
}

// SDL音频回调函数
void audioCallback(void *userdata, Uint8 *stream, int len) {
    AVPacket packet;
    av_init_packet(&packet);
    packet.data = nullptr;
    packet.size = 0;

    int dataSize = 0;

    while (dataSize < len) {
        // 检查音频包队列是否为空
        if (audioQueue.size <= 0) {
            break;
        }

        // 出队音频包
        dequeuePacket(&audioQueue, &packet);
        // 解码音频包
        int ret = avcodec_send_packet(codecContext, &packet);
        if (ret < 0) {
            break;
        }

        ret = avcodec_receive_frame(codecContext, frame);
        if (ret < 0) {
            break;
        }

        // 计算音频帧数据大小
        int frameDataSize = av_samples_get_buffer_size(nullptr, codecContext->channels,
                                                    frame->nb_samples,
                                                    codecContext->sample_fmt, 1);

        // 将音频帧数据写入音频设备或缓冲区
        memcpy(stream + dataSize, frame->data[0], frameDataSize);
        dataSize += frameDataSize;

        av_frame_unref(frame);
    }

    av_packet_unref(&packet);

    // 如果音频数据不足，填充空白数据
    if (dataSize < len) {
        memset(stream + dataSize, 0, len - dataSize);
        dataSize = len;
    }

    // 更新音频时钟
    if (!audioClock.paused) {
        audioClock.pts += (double)dataSize / (2 * codecContext->channels * codecContext->sample_rate);
    }
}

// 初始化音频时钟
void initAudioClock() {
    audioClock.pts = 0.0;
    audioClock.speed = 1.0;
    audioClock.paused = 0;
    audioClock.basePts = 0;
    audioClock.mutex = SDL_CreateMutex();
}

// 销毁音频时钟
void destroyAudioClock() {
    SDL_DestroyMutex(audioClock.mutex);
}

// 设置音频时钟的基准时间
void setAudioClockBasePts(int64_t basePts) {
    SDL_LockMutex(audioClock.mutex);
    audioClock.basePts = basePts;
    SDL_UnlockMutex(audioClock.mutex);
}

// 设置音频时钟的速度
void setAudioClockSpeed(double speed) {
    SDL_LockMutex(audioClock.mutex);
    audioClock.speed = speed;
    SDL_UnlockMutex(audioClock.mutex);
}

// 播放音频
void playAudio() {
    SDL_PauseAudioDevice(audioDeviceID, 0);
}

// 暂停音频
void pauseAudio() {
    SDL_PauseAudioDevice(audioDeviceID, 1);
}

// 读取音频包线程
int readPacketThread(void *arg) {
    AVPacket packet;
    av_init_packet(&packet);
    packet.data = nullptr;
    packet.size = 0;
    while (av_read_frame(formatContext, &packet) >= 0) {
    // 入队音频包
    if (packet.stream_index == audioStreamIndex) {
        enqueuePacket(&audioQueue, &packet);
    }

    av_packet_unref(&packet);
    }
    return 0;
}

int main(int argc,char** argv) {
    //av_register_all();
    avformat_network_init();
    SDL_Init(SDL_INIT_AUDIO);
    // 打开音频文件
    if (avformat_open_input(&formatContext, argv[1], nullptr, nullptr) != 0) {
        std::cerr << "Failed to open audio file" << std::endl;
        return EXIT_FAILURE;
    }

    // 查找音频流信息
    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        std::cerr << "Failed to find stream information" << std::endl;
        avformat_close_input(&formatContext);
        return EXIT_FAILURE;
    }

    // 查找音频流索引
    audioStreamIndex = -1;
    for (unsigned int i = 0; i < formatContext->nb_streams; ++i) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStreamIndex = i;
            break;
        }
    }
    if (audioStreamIndex == -1) {
    std::cerr << "Failed to find audio stream" << std::endl;
    avformat_close_input(&formatContext);
    return EXIT_FAILURE;
}

    AVCodec *codec = avcodec_find_decoder(formatContext->streams[audioStreamIndex]->codecpar->codec_id);
    codecContext = avcodec_alloc_context3(codec);
    if (avcodec_open2(codecContext, codec, nullptr) < 0) {
        std::cerr << "Failed to open audio codec" << std::endl;
        avcodec_free_context(&codecContext);
        avformat_close_input(&formatContext);
        return EXIT_FAILURE;
    }

    // 初始化音频包队列
    initPacketQueue(&audioQueue);

    // 初始化音频时钟
    initAudioClock();

    // 配置SDL音频参数
    SDL_AudioSpec desiredSpec;
    memset(&desiredSpec, 0, sizeof(desiredSpec));
    desiredSpec.freq = codecContext->sample_rate;
    desiredSpec.format = AUDIO_S16SYS;
    desiredSpec.channels = codecContext->channels;
    desiredSpec.samples = codecContext->frame_size;
    desiredSpec.callback = audioCallback;

    frame=av_frame_alloc();

    // 打开音频设备
    audioDeviceID = SDL_OpenAudio(&desiredSpec, NULL);
    if (audioDeviceID < 0) {
        std::cerr << "Failed to open audio device" << std::endl;
        avcodec_free_context(&codecContext);
        avformat_close_input(&formatContext);
        return EXIT_FAILURE;
    }

    // 创建读取音频包线程
    SDL_Thread *readPacketThreadID = SDL_CreateThread(readPacketThread, "ReadPacketThread", nullptr);

    // 读取外部时钟时间
    int64_t externalClockBasePts = av_gettime_relative();
    int64_t externalClockPausedPts = 0;
    int externalClockPaused = 0;

    // 播放音频
    playAudio();




    // 主循环
    bool quit = false;
    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
                break;
            }
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_SPACE) {
                    if (audioClock.paused) {
                        // 恢复播放
                        setAudioClockBasePts(audioClock.basePts + av_gettime_relative() - audioClock.pausedPts);
                        setAudioClockSpeed(audioClock.speed);
                        audioClock.paused = 0;
                        playAudio();
                    }
                    else {
                        // 暂停播放
                        pauseAudio();
                        audioClock.pausedPts = av_gettime_relative();
                        audioClock.paused = 1;
                    }
                }
                else if (event.key.keysym.sym == SDLK_LEFT) {
                    // 从5s前的packet开始播放
                    int64_t targetPts = audioClock.basePts - 5 * AV_TIME_BASE;
                    if (targetPts < 0) {
                        targetPts = 0;
                    }
                    setAudioClockBasePts(targetPts);
                    setAudioClockSpeed(audioClock.speed);
                }
                else if (event.key.keysym.sym == SDLK_RIGHT) {
                    // 从5s后的packet开始播放
                    int64_t targetPts = audioClock.basePts + 5 * AV_TIME_BASE;
                    setAudioClockBasePts(targetPts);
                    setAudioClockSpeed(audioClock.speed);
                }
                else if (event.key.keysym.sym == SDLK_UP) {
                    // 加速播放
                    audioClock.speed *= 2.0;
                    setAudioClockSpeed(audioClock.speed);
                }
                else if (event.key.keysym.sym == SDLK_DOWN) {
                    // 减速播放
                    audioClock.speed /= 2.0;
                    setAudioClockSpeed(audioClock.speed);
                }
            }
        }

        // 更新外部时钟时间
        int64_t externalClockCurrentPts = av_gettime_relative();
        if (!externalClockPaused) {
            externalClockBasePts += externalClockCurrentPts - externalClockPausedPts;
        }
        externalClockPausedPts = externalClockCurrentPts;

        // 同步音频到外部时钟
        int64_t externalClockDiff = audioClock.basePts - externalClockBasePts;
        double externalClockDiffSeconds = (double)externalClockDiff / AV_TIME_BASE;
        double audioSpeed = audioClock.speed;
        double threshold = 0.1;

        // 调整音频时钟速度
        if (externalClockDiffSeconds > threshold) {
            audioSpeed += 0.1;
        }
        else if (externalClockDiffSeconds < -threshold) {
            audioSpeed -= 0.1;
        }
        setAudioClockSpeed(audioSpeed);

        // 输出音频时钟和外部时钟时间差
        std::cout << "Audio Clock: " << audioClock.pts << ", External Clock: "
                << (double)externalClockBasePts / AV_TIME_BASE << std::endl;

        SDL_Delay(10);
    }

    // 清理资源
    destroyPacketQueue(&audioQueue);
    destroyAudioClock();
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    SDL_CloseAudioDevice(audioDeviceID);
    SDL_Quit();

    return EXIT_SUCCESS;
}

