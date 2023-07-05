#include "packetQueue.h"
#include "control.h"

//int64_t myAVPacket::num=0;//从0开始对myAVPacket计数，第一个包序号为1

int packetQueue::packet_queue_push(std::shared_ptr<myAVPacket> pkt_ptr)
{
    std::unique_lock<std::mutex> lock(Mutex);

    size += pkt_ptr->mypkt.size;
    pkts_ptr.push_back(pkt_ptr);
    lock.unlock();
    cond.notify_one();
    pkt_ptr->is_recived=false;
    
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

void packetQueue::insert(int64_t pos, char* data){
    std::unique_lock<std::mutex> lock(Mutex);
    if(pos>=pkts_ptr.size()||pos<0)
        throw std::runtime_error("queue insert out of range");
    pkts_ptr[pos]->mypkt.data=(uint8_t*)data;
    pkts_ptr[pos]->is_recived=true;
    lock.unlock();
    cond.notify_all();
}

void packetQueue::seek(int64_t& timeshaft,double timebase){
    std::unique_lock<std::mutex> lock(Mutex);

    int64_t dts=pkts_ptr[curr_decode_pos]->mypkt.dts*timebase;

    if(timeshaft>dts){
        while(timeshaft>dts){
            curr_decode_pos++;
            if(curr_decode_pos>=pkts_ptr.size()){curr_decode_pos=pkts_ptr.size()-1;break;}
            if(pkts_ptr[curr_decode_pos]->mypkt.flags!=1)continue;
            dts=pkts_ptr[curr_decode_pos]->mypkt.dts*timebase;
        }
        if(!pkts_ptr[curr_decode_pos]->is_recived){
            pause();
            seek_callback(pkts_ptr[curr_decode_pos]->num);
            cond.wait(lock, [&]{ return pkts_ptr[curr_decode_pos]->is_recived;});
            //lock.unlock();while(!pkts_ptr[curr_decode_pos]->is_recived){SDL_Delay(1);}lock.lock();
            action();
        }
        
    }
    else{
        while(timeshaft<dts){
            curr_decode_pos--;
            if(curr_decode_pos<0){curr_decode_pos=0;break;}
            if(pkts_ptr[curr_decode_pos]->mypkt.flags!=1)continue;
            dts=pkts_ptr[curr_decode_pos]->mypkt.dts*timebase;
        }
        if(!pkts_ptr[curr_decode_pos]->is_recived){
            pause();
            seek_callback(pkts_ptr[curr_decode_pos]->num);
            cond.wait(lock, [&]{ return pkts_ptr[curr_decode_pos]->is_recived;});
            //lock.unlock();while(!pkts_ptr[curr_decode_pos]->is_recived){SDL_Delay(1);}lock.lock();
            action();
        }
    }
    timeshaft=pkts_ptr[curr_decode_pos]->mypkt.dts*timebase;
    lock.unlock();
}