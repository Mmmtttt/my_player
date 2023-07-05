#include "packetQueue.h"
#include "control.h"

//int64_t myAVPacket::num=0;//从0开始对myAVPacket计数，第一个包序号为1

int packetQueue::packet_queue_push(std::shared_ptr<myAVPacket> pkt_ptr)
{
    std::unique_lock<std::mutex> lock(Mutex);

    size += pkt_ptr->mypkt.size;
    pkts_ptr.push_back(pkt_ptr);

    cond.notify_one();
    lock.unlock();
    return 0;
}

int packetQueue::packet_queue_pop(std::shared_ptr<myAVPacket>& pkt_ptr, int block)
{
    int ret;

    //SDL_LockMutex(q->mutex);
    std::unique_lock<std::mutex> lock(Mutex);
    while (1)
    {
        if(curr_decode_pos<0){curr_decode_pos=0;}
        else if (curr_decode_pos<pkts_ptr.size())             // 队列非空，取一个出来
        {
            if(!pkts_ptr[curr_decode_pos]->is_recived){
                pause();
                cond.wait(lock, [&]{ return pkts_ptr[curr_decode_pos]->is_recived;});
                action();
            }
            pkt_ptr=pkts_ptr[curr_decode_pos];
            curr_decode_pos++;
            ret = 1;
            break;
        }
        
        else                        // 队列空且阻塞标志有效，则等待
        {
            throw std::runtime_error("queue pop out of range");
        }
    }
    lock.unlock();
    return ret;
}

void packetQueue::insert(std::shared_ptr<myAVPacket> pkt_ptr){
    std::unique_lock<std::mutex> lock(Mutex);
    if(pkt_ptr->id_in_queue>=pkts_ptr.size()||pkt_ptr->id_in_queue<0)
        throw std::runtime_error("queue insert out of range");
    pkts_ptr[pkt_ptr->id_in_queue]=pkt_ptr;
    size+=pkt_ptr->mypkt.size;
    pkt_ptr->is_recived=true;
    lock.unlock();
    cond.notify_all();
}

void packetQueue::seek(int64_t& timeshaft,double timebase){
    std::unique_lock<std::mutex> lock(Mutex);

    int64_t dts=pkts_ptr[curr_decode_pos]->mypkt.dts*timebase;

    if(timeshaft>dts){
        while(timeshaft>dts){
            curr_decode_pos++;
            if(curr_decode_pos>=pkts_ptr.size()){curr_decode_pos=pkts_ptr.size()-1;lock.unlock();}
            if(!pkts_ptr[curr_decode_pos]->is_recived){
                pause();
                cond.wait(lock, [&]{ return pkts_ptr[curr_decode_pos]->is_recived;});
                action();
            }
            dts=pkts_ptr[curr_decode_pos]->mypkt.dts*timebase;
        }
    }
    else{
        while(timeshaft<dts){
            curr_decode_pos--;
            if(curr_decode_pos<0){curr_decode_pos=0;lock.unlock();}
            if(!pkts_ptr[curr_decode_pos]->is_recived){
                pause();
                cond.wait(lock, [&]{ return pkts_ptr[curr_decode_pos]->is_recived;});
                action();
            }
            dts=pkts_ptr[curr_decode_pos]->mypkt.dts*timebase;
        }
    }
    lock.unlock();
}