#ifndef PACKETQUEUE_H
#define PACKETQUEUE_H

#include <libavcodec/avcodec.h>
#include <list>
#include <mutex>
#include <condition_variable>
#include <iostream>
class myAVPacket{
    public:
        myAVPacket(){}
        myAVPacket(AVPacket pkt):mypkt(pkt),size(pkt.size){}
        ~myAVPacket(){
            //std::cout<<"destory num "<<num<<std::endl;
            av_packet_unref(&mypkt);
        }
        AVPacket mypkt;
        int size;
        //static int num;
};

class packetQueue{
    public:
        packetQueue(){std::cout<<"packet queue create"<<std::endl;}
        ~packetQueue(){pkts_ptr.clear();std::cout<<"packet queue destoryed"<<std::endl;}


        int packet_queue_push(std::unique_ptr<myAVPacket> pkt_ptr);
        int packet_queue_pop(std::unique_ptr<myAVPacket>& pkt_ptr, int block);
    private:
        std::list<std::unique_ptr<myAVPacket>> pkts_ptr;
        int size=0;         // 队列中AVPacket总的大小(字节数)
        std::mutex Mutex;
        std::condition_variable cond;
};



#endif