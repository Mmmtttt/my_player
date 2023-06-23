#ifndef PACKETQUEUE_H
#define PACKETQUEUE_H

#include <libavcodec/avcodec.h>
#include <list>
#include <mutex>
#include <condition_variable>
#include <iostream>

class packetQueue{
    public:
        packetQueue(){std::cout<<"packet queue create"<<std::endl;}
        ~packetQueue(){pkts.clear();std::cout<<"packet queue destoryed"<<std::endl;}


        int packet_queue_push(AVPacket *pkt);
        int packet_queue_pop(AVPacket *pkt, int block);
    private:
        std::list<AVPacket> pkts;
        int size=0;         // 队列中AVPacket总的大小(字节数)
        std::mutex Mutex;
        std::condition_variable cond;
};



#endif