#ifndef PACKETQUEUE_H
#define PACKETQUEUE_H

#include <libavcodec/avcodec.h>
#include <list>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <deque>
#include <iostream>
#include <map>
class myAVPacket{
    public:
        myAVPacket(){}
        //myAVPacket(AVPacket pkt):mypkt(pkt),size(pkt.size){num++;}
        ~myAVPacket(){
            //std::cout<<"destory num "<<num<<std::endl;
            av_packet_unref(&mypkt);
        }

        AVPacket mypkt;
        int64_t size;
        int64_t num=0;//从文件中取出的packet的序号，无论什么类型
        int64_t id_in_queue=-1;//在某个队列中的packet序号，从-1开始是为了和deque对应
        bool is_recived=false;
        bool is_sended=false;
};

class packetQueue{
    public:
        packetQueue(){std::cout<<"packet queue create"<<std::endl;}
        ~packetQueue(){pkts_ptr.clear();std::cout<<"packet queue destoryed"<<std::endl;}

        void initial(int64_t size){
            for(int64_t i=0;i<size;i++){
                std::shared_ptr<myAVPacket> temp=std::shared_ptr<myAVPacket>(new myAVPacket);
                pkts_ptr.push_back(temp);
            }
            std::cout<<"packet queue resize "<<size<<std::endl;
        }
        void insert(std::shared_ptr<myAVPacket> pkt_ptr);

        int packet_queue_push(std::shared_ptr<myAVPacket> pkt_ptr);
        int packet_queue_pop(std::shared_ptr<myAVPacket>& pkt_ptr, int block);
        void seek(int64_t& timeshaft,double timebase);//timeshaft加引用是为了实时获取此刻timeshaft的值，对于音频和视频，timebase通常不同
    //private:
        std::deque<std::shared_ptr<myAVPacket>> pkts_ptr;
        int64_t size=0;         // 队列中AVPacket总的大小(字节数)
        std::mutex Mutex;
        std::condition_variable cond;
        int64_t curr_decode_pos=0;
};

extern packetQueue video_packet_queue;
extern packetQueue audio_packet_queue;

#endif