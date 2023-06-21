#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include <SDL.h>
#include <SDL_thread.h>

#ifdef __MINGW32__
#undef main /* Prevents SDL from overriding main() */
#endif

#include <stdio.h>
#include <assert.h>

#define SDL_AUDIO_BUFFER_SIZE 1024
#define MAX_AUDIO_FRAME_SIZE 192000

typedef struct MyAVPacketList {
  AVPacket pkt;
  struct MyAVPacketList *next;
} MyPacketList;
typedef struct PacketQueue {
  MyPacketList *first_pkt, *last_pkt;
  int nb_packets;
  int size;
  SDL_mutex *mutex;
  SDL_cond *cond;
} PacketQueue;

PacketQueue audioq;

int quit = 0;

void packet_queue_init(PacketQueue *q) {
  memset(q, 0, sizeof(PacketQueue));
  q->mutex = SDL_CreateMutex();
  q->cond = SDL_CreateCond();
}

int packet_queue_put(PacketQueue *q, AVPacket *pkt) {
  MyPacketList *pkt1 = (MyPacketList *)malloc(sizeof(MyPacketList));
  if (!pkt1) {
    return -1;
  }
  if (av_packet_ref(&pkt1->pkt, pkt) < 0) {
    //av_packet_list_free(&pkt1);
    return -1;
  }
  pkt1->next = NULL;

  SDL_LockMutex(q->mutex);

  if (!q->last_pkt) {
    q->first_pkt = pkt1;
  } else {
    q->last_pkt->next = pkt1;
  }
  q->last_pkt = pkt1;
  q->nb_packets++;
  q->size += pkt1->pkt.size;
  SDL_CondSignal(q->cond);

  SDL_UnlockMutex(q->mutex);
  return 0;
}

static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block) {
  MyPacketList *pkt1;
  int ret;

  SDL_LockMutex(q->mutex);

  for (;;) {
    if (quit) {
      ret = -1;
      break;
    }

    pkt1 = q->first_pkt;
    if (pkt1) {
      q->first_pkt = pkt1->next;
      if (!q->first_pkt) {
        q->last_pkt = NULL;
      }
      q->nb_packets--;
      q->size -= pkt1->pkt.size;
      *pkt = pkt1->pkt;
      av_packet_list_free(&pkt1);
      ret = 1;
      break;
    } else if (!block) {
      ret = 0;
      break;
    } else {
      SDL_CondWait(q->cond, q->mutex);
    }
  }
  SDL_UnlockMutex(q->mutex);
  return ret;
}

int audio_decode_frame(AVCodecContext *aCodecCtx, uint8_t *audio_buf, int buf_size) {
  static AVPacket pkt;
  static uint8_t *audio_pkt_data = NULL;
  static int audio_pkt_size = 0;
  static AVFrame *frame = NULL;

  int len1, data_size = 0;

  for (;;) {
    while (audio_pkt_size > 0) {
      int got_frame = 0;
      len1 = avcodec_receive_frame(aCodecCtx, frame);
      if (len1 == AVERROR(EAGAIN) || len1 == AVERROR_EOF) {
        break;
      } else if (len1 < 0) {
        audio_pkt_size = 0;
        break;
      }
      audio_pkt_data += len1;
      audio_pkt_size -= len1;
      data_size = 0;
      if (got_frame) {
        data_size = av_samples_get_buffer_size(
            NULL, aCodecCtx->channel_layout, frame->nb_samples, aCodecCtx->sample_fmt, 1);
        assert(data_size <= buf_size);
        memcpy(audio_buf, frame->data[0], data_size);
      }
      if (data_size <= 0) {
        continue;
      }
      return data_size;
    }
    if (pkt.data) {
      av_packet_unref(&pkt);
    }

    if (quit) {
      return -1;
    }

    if (packet_queue_get(&audioq, &pkt, 1) < 0) {
      return -1;
    }
    audio_pkt_data = pkt.data;
    audio_pkt_size = pkt.size;
  }
}

void audio_callback(void *userdata, Uint8 *stream, int len) {
  AVCodecContext *aCodecCtx = (AVCodecContext *)userdata;
  int len1, audio_size;

  static uint8_t audio_buf[(MAX_AUDIO_FRAME_SIZE * 3) / 2];
  static unsigned int audio_buf_size = 0;
  static unsigned int audio_buf_index = 0;

  while (len > 0) {
    if (audio_buf_index >= audio_buf_size) {
      audio_size = audio_decode_frame(aCodecCtx, audio_buf, sizeof(audio_buf));
      if (audio_size < 0) {
        audio_buf_size = 1024;  // arbitrary?
        memset(audio_buf, 0, audio_buf_size);
      } else {
        audio_buf_size = audio_size;
      }
      audio_buf_index = 0;
    }
    len1 = audio_buf_size - audio_buf_index;
    if (len1 > len) {
      len1 = len;
    }
    memcpy(stream, (uint8_t *)audio_buf + audio_buf_index, len1);
    len -= len1;
    stream += len1;
    audio_buf_index += len1;
  }
}

int main(int argc, char *argv[]) {
  AVFormatContext *pFormatCtx = NULL;
  int i, videoStream, audioStream;
  AVCodecParameters *p_codec_par = NULL;
  AVCodecContext *pCodecCtx = NULL;
  const AVCodec *pCodec = NULL;
  AVFrame *pFrame = NULL;
  AVPacket packet;
  int frameFinished;
  struct SwsContext *sws_ctx = NULL;

  AVCodecParameters *a_codec_par = NULL;
  AVCodecContext *aCodecCtx = NULL;
  const AVCodec *aCodec = NULL;

  SDL_Window *screen;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  SDL_Event event;
  SDL_AudioSpec wanted_spec, spec;

  if (argc < 2) {
    fprintf(stderr, "Usage: test <file>\n");
    exit(1);
  }

  avformat_open_input(&pFormatCtx, argv[1], NULL, NULL);
  avformat_find_stream_info(pFormatCtx, NULL);
  av_dump_format(pFormatCtx, 0, argv[1], 0);

  videoStream = -1;
  audioStream = -1;
  for (i = 0; i < pFormatCtx->nb_streams; i++) {
    if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO &&
        videoStream < 0) {
      videoStream = i;
    }
    if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO &&
        audioStream < 0) {
      audioStream = i;
    }
  }
  if (videoStream == -1) {
    return -1;  // Didn't find a video stream
  }
  if (audioStream == -1) {
    return -1;  // Didn't find an audio stream
  }

  a_codec_par = pFormatCtx->streams[audioStream]->codecpar;
  aCodec = avcodec_find_decoder(a_codec_par->codec_id);
  if (!aCodec) {
    fprintf(stderr, "Unsupported audio codec!\n");
    return -1;
  }

  aCodecCtx = avcodec_alloc_context3(aCodec);
  if (!aCodecCtx) {
    fprintf(stderr, "Failed to allocate audio codec context!\n");
    return -1;
  }

  if (avcodec_parameters_to_context(aCodecCtx, a_codec_par) < 0) {
    fprintf(stderr, "Failed to copy audio codec parameters to context!\n");
    return -1;
  }

  if (avcodec_open2(aCodecCtx, aCodec, NULL) < 0) {
    fprintf(stderr, "Failed to open audio codec!\n");
    return -1;
  }

  packet_queue_init(&audioq);
  //SDL_AudioSpec wanted_spec;
  wanted_spec.freq = aCodecCtx->sample_rate;
  wanted_spec.format = AUDIO_S16SYS;
  wanted_spec.channels = aCodecCtx->channel_layout;
  wanted_spec.silence = 0;
  wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;
  wanted_spec.callback = audio_callback;
  wanted_spec.userdata = aCodecCtx;

  if (SDL_OpenAudio(&wanted_spec, &spec) < 0) {
    fprintf(stderr, "Failed to open audio device: %s\n", SDL_GetError());
    return -1;
  }
  SDL_PauseAudio(0);

  p_codec_par = pFormatCtx->streams[videoStream]->codecpar;
  pCodec = avcodec_find_decoder(p_codec_par->codec_id);
  if (!pCodec) {
    fprintf(stderr, "Unsupported video codec!\n");
    return -1;
  }

  pCodecCtx = avcodec_alloc_context3(pCodec);
  if (!pCodecCtx) {
    fprintf(stderr, "Failed to allocate video codec context!\n");
    return -1;
  }

  if (avcodec_parameters_to_context(pCodecCtx, p_codec_par) < 0) {
    fprintf(stderr, "Failed to copy video codec parameters to context!\n");
    return -1;
  }

  if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
    fprintf(stderr, "Failed to open video codec!\n");
    return -1;
  }

  pFrame = av_frame_alloc();
  if (!pFrame) {
    fprintf(stderr, "Failed to allocate video frame!\n");
    return -1;
  }

  screen = SDL_CreateWindow("Video Player", SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED, pCodecCtx->width,
                            pCodecCtx->height, SDL_WINDOW_SHOWN);
  if (!screen) {
    fprintf(stderr, "Failed to create SDL window: %s\n", SDL_GetError());
    return -1;
  }

  renderer = SDL_CreateRenderer(screen, -1, 0);
  if (!renderer) {
    fprintf(stderr, "Failed to create SDL renderer: %s\n", SDL_GetError());
    return -1;
  }

  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12,
                              SDL_TEXTUREACCESS_STREAMING, pCodecCtx->width,
                              pCodecCtx->height);
  if (!texture) {
    fprintf(stderr, "Failed to create SDL texture: %s\n", SDL_GetError());
    return -1;
  }

  sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width,
                           pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);

  while (av_read_frame(pFormatCtx, &packet) >= 0) {
    if (packet.stream_index == videoStream) {
      avcodec_send_packet(pCodecCtx, &packet);
      while (avcodec_receive_frame(pCodecCtx, pFrame) == 0) {
        SDL_UpdateYUVTexture(texture, NULL, pFrame->data[0], pFrame->linesize[0], pFrame->data[1],
                             pFrame->linesize[1], pFrame->data[2], pFrame->linesize[2]);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
      }
    } else if (packet.stream_index == audioStream) {
      packet_queue_put(&audioq, &packet);
    }
    av_packet_unref(&packet);

    SDL_PollEvent(&event);
    switch (event.type) {
      case SDL_QUIT:
        quit = 1;
        break;
      default:
        break;
    }

    if (quit) {
      break;
    }
  }

  av_frame_free(&pFrame);
  avcodec_free_context(&pCodecCtx);
  avcodec_free_context(&aCodecCtx);
  avformat_close_input(&pFormatCtx);

  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(screen);
  SDL_CloseAudio();

  return 0;
}
