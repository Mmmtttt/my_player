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
        myAVPacket(AVPacket pkt):mypkt(pkt),size(pkt.size){num++;}
        ~myAVPacket(){
            //std::cout<<"destory num "<<num<<std::endl;
            av_packet_unref(&mypkt);
        }

        AVPacket mypkt;
        int size;
        static int64_t num;//从文件中取出的packet的序号，无论什么类型
        bool is_recived=false;
        bool is_sended=false;
};

class packetQueue{
    public:
        packetQueue(){std::cout<<"packet queue create"<<std::endl;}
        ~packetQueue(){pkts_ptr.clear();std::cout<<"packet queue destoryed"<<std::endl;}

        void initial(int64_t size){pkts_ptr.resize(size);std::cout<<"packet queue resize "<<size<<std::endl;}
        void insert(std::shared_ptr<myAVPacket> pkt_ptr){
            pkts_ptr[pkt_ptr->num-1]=pkt_ptr;
            size+=pkt_ptr->mypkt.size;
            pkt_ptr->is_recived=true;
        }

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