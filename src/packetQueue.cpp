#include "packetQueue.h"
#include "control.h"

//int64_t myAVPacket::num=0;//从0开始对myAVPacket计数，第一个包序号为1

int packetQueue::packet_queue_push(std::shared_ptr<myAVPacket> pkt_ptr)
{
    std::unique_lock<std::mutex> lock(Mutex);

    size += pkt_ptr->mypkt.size;
    pkts_ptr.push_back(pkt_ptr);
    pkt_ptr->is_recived=false;
    lock.unlock();
    cond.notify_all();
    
    
    return 0;
}

int packetQueue::packet_queue_pop(std::shared_ptr<myAVPacket>& pkt_ptr, int block)
{
    int ret;

    //SDL_LockMutex(q->mutex);
    std::unique_lock<std::mutex> lock(Mutex);
    //std::cout<<"get lock"<<pkts_ptr[0]->mypkt.stream_index<<" from pop"<<std::endl;
    while (1)
    {
        if(curr_decode_pos<0){curr_decode_pos=0;pause();}
        else if (curr_decode_pos<pkts_ptr.size())             
        {
            if(!pkts_ptr[curr_decode_pos]->is_recived){
                pause();
                seek_callback(pkts_ptr[0]->mypkt.stream_index,pkts_ptr[curr_decode_pos]->id_in_queue);
                cond.wait(lock, [&]{ return pkts_ptr[curr_decode_pos]->is_recived;});
                action();
            }
            pkt_ptr=pkts_ptr[curr_decode_pos];
            curr_decode_pos++;
            ret = 1;
            break;
        }
        
        else                        
        {
            time_shaft-=1500;
            curr_decode_pos=pkts_ptr.size()-1;
            pkt_ptr=pkts_ptr[curr_decode_pos];
            pause();
            break;
        }
    }
    lock.unlock();
    //std::cout<<"free lock"<<pkts_ptr[0]->mypkt.stream_index<<" from pop"<<std::endl;
    return ret;
}

bool packetQueue::insert(std::shared_ptr<myAVPacket> pkt_ptr){
    //std::unique_lock<std::mutex> lock(Mutex);
    if(pkt_ptr->id_in_queue>=pkts_ptr.size()||pkt_ptr->id_in_queue<0)
        return false;
    pkts_ptr[pkt_ptr->id_in_queue]=pkt_ptr;
    size+=pkt_ptr->mypkt.size;
    pkt_ptr->is_recived=true;
    //lock.unlock();
    cond.notify_all();
    return true;
}

bool packetQueue::insert(int64_t pos, char* data){
    //std::unique_lock<std::mutex> lock(Mutex);
    if(pos>=pkts_ptr.size()||pos<0)
        return false;
    pkts_ptr[pos]->mypkt.data=(uint8_t*)data;
    pkts_ptr[pos]->is_recived=true;
    //lock.unlock();
    cond.notify_all();
    return true;
}

void packetQueue::seek(int64_t& timeshaft,double timebase){
    static std::mutex seek_Mutex;
    
    std::unique_lock<std::mutex> seek_lock(seek_Mutex);
    //std::cout<<"get seek_lock queue "<<pkts_ptr[0]->mypkt.stream_index<<std::endl;
    std::unique_lock<std::mutex> lock(Mutex);
    //std::cout<<"get lock"<<pkts_ptr[0]->mypkt.stream_index<<" from seek"<<std::endl;

    pause();
    int64_t dts;
    if(curr_decode_pos>=pkts_ptr.size()){curr_decode_pos=pkts_ptr.size()-1;return;}
    dts=pkts_ptr[curr_decode_pos]->mypkt.dts*timebase;

    if(timeshaft>dts){
        while(timeshaft>dts){
            curr_decode_pos++;
            if(curr_decode_pos>=pkts_ptr.size()){curr_decode_pos=pkts_ptr.size()-1;break;}
            if((pkts_ptr[curr_decode_pos]->mypkt.stream_index==0)&&(pkts_ptr[curr_decode_pos]->mypkt.flags!=1))continue;
            dts=pkts_ptr[curr_decode_pos]->mypkt.dts*timebase;
        }
    }
    else{
        while(timeshaft<dts){
            curr_decode_pos--;
            if(curr_decode_pos<0){curr_decode_pos=0;break;}
            if((pkts_ptr[curr_decode_pos]->mypkt.stream_index==0)&&(pkts_ptr[curr_decode_pos]->mypkt.flags!=1))continue;
            dts=pkts_ptr[curr_decode_pos]->mypkt.dts*timebase;
        }
        
    }
    
    int64_t curr_decode_pos_copy=curr_decode_pos;
    
    if(!pkts_ptr[curr_decode_pos_copy]->is_recived){
        
        std::cout<<"waiting queue "<<pkts_ptr[0]->mypkt.stream_index<<"  packet  "<<pkts_ptr[curr_decode_pos_copy]->id_in_queue<<std::endl;
        seek_callback(pkts_ptr[0]->mypkt.stream_index,pkts_ptr[curr_decode_pos_copy]->id_in_queue);
        cond.wait(lock, [&]{ return pkts_ptr[curr_decode_pos_copy]->is_recived;});
        std::cout<<" queue "<<pkts_ptr[0]->mypkt.stream_index<<"  packet  "<<pkts_ptr[curr_decode_pos_copy]->id_in_queue<<" received"<<std::endl;
    }

    

    if(!pkts_ptr[0]->mypkt.stream_index)
        timeshaft=pkts_ptr[curr_decode_pos]->mypkt.dts*timebase;
    
    
    lock.unlock();
    //std::cout<<"free lock"<<pkts_ptr[0]->mypkt.stream_index<<" from seek"<<std::endl;
    seek_lock.unlock();
    //std::cout<<"free seek_lock queue "<<pkts_ptr[0]->mypkt.stream_index<<std::endl;
    action();
}