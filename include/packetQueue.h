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
        myAVPacket(){is_exist=false;}
        myAVPacket(AVPacket pkt):mypkt(pkt),size(pkt.size){num++;is_exist=true;}
        ~myAVPacket(){
            //std::cout<<"destory num "<<num<<std::endl;
            av_packet_unref(&mypkt);
        }

        AVPacket mypkt;
        int size;
        static int64_t num;//从文件中取出的packet的序号，无论什么类型
        bool is_exist=false;
};

class packetQueue{
    public:
        packetQueue(){std::cout<<"packet queue create"<<std::endl;}
        ~packetQueue(){pkts_ptr.clear();std::cout<<"packet queue destoryed"<<std::endl;}


        int packet_queue_push(std::shared_ptr<myAVPacket> pkt_ptr);
        int packet_queue_pop(std::shared_ptr<myAVPacket>& pkt_ptr, int block);
        void seek(int64_t timeshaft,double timebase){
            std::unique_lock<std::mutex> lock(Mutex);

            int64_t dts=pkts_ptr[curr_decode_pos]->mypkt.dts*timebase;

            if(timeshaft>dts){
                while(timeshaft>dts){
                    curr_decode_pos++;
                    if(curr_decode_pos>=pkts_ptr.size()){curr_decode_pos=pkts_ptr.size()-1;return;}
                    dts=pkts_ptr[curr_decode_pos]->mypkt.dts*timebase;
                }
            }
            else{
                while(timeshaft<dts){
                    curr_decode_pos--;
                    if(curr_decode_pos<0){curr_decode_pos=0;return;}
                    dts=pkts_ptr[curr_decode_pos]->mypkt.dts*timebase;
                }
            }
            lock.unlock();
        }
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