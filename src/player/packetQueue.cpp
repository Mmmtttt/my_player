#include "packetQueue.h"
#include "control.h"
#include <windows.h>

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

    std::unique_lock<std::mutex> lock(Mutex);
    while (1)
    {
        if(curr_decode_pos<0){curr_decode_pos=0;pause();}
        else if (curr_decode_pos<pkts_ptr.size())             
        {
            if(!is_curr_received()){
                pause();
                std::cout<<"waiting queue "<<get_idx()<<"  packet  "<<get_curr_id()<<std::endl;
                seek_callback(get_idx(),get_curr_id());
                cond.wait(lock, [&]{ return is_curr_received();});
                std::cout<<" queue "<<get_idx()<<"  packet  "<<get_curr_id()<<" received"<<std::endl;
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
    return ret;
}

bool packetQueue::insert(std::shared_ptr<myAVPacket> pkt_ptr){
    if(pkt_ptr->id_in_queue>=pkts_ptr.size()||pkt_ptr->id_in_queue<0)
        return false;
    //pkts_ptr[pkt_ptr->id_in_queue].reset();
    pkts_ptr[pkt_ptr->id_in_queue]=pkt_ptr;
    size+=pkt_ptr->mypkt.size;
    pkt_ptr->is_recived=true;
    //lock.unlock();
    cond.notify_all();
    return true;
}

bool packetQueue::insert(int64_t pos, char* data){
    if(pos>=pkts_ptr.size()||pos<0)
        return false;
    pkts_ptr[pos]->mypkt.data=(uint8_t*)data;
    pkts_ptr[pos]->is_recived=true;
    cond.notify_all();
    return true;
}

void packetQueue::seek(int64_t& timeshaft,double timebase){
    static std::mutex seek_Mutex;
    
    std::unique_lock<std::mutex> seek_lock(seek_Mutex);
    std::unique_lock<std::mutex> lock(Mutex);

    pause();
    int64_t dts;
    if(curr_decode_pos>=get_pkt_count()){get_pkt_count()-1;return;}
    dts=get_curr_dts()*timebase;

    if(timeshaft>dts){
        while(timeshaft>dts){
            curr_decode_pos++;
            if(curr_decode_pos>=get_pkt_count()){get_pkt_count()-1;break;}
            if((get_idx()==0)&&(get_curr_flag()!=1))continue;
            dts=get_curr_dts()*timebase;
        }
    }
    else{
        while(timeshaft<dts){
            curr_decode_pos--;
            if(curr_decode_pos<0){curr_decode_pos=0;break;}
            if((get_idx()==0)&&(get_curr_flag()!=1))continue;
            dts=get_curr_dts()*timebase;
        }
        
    }

    if(!is_curr_received()){
        
        std::cout<<"waiting queue "<<get_idx()<<"  packet  "<<get_curr_id()<<std::endl;
        seek_callback(get_idx(),get_curr_id());
        cond.wait(lock, [&]{ return is_curr_received();});
        std::cout<<" queue "<<get_idx()<<"  packet  "<<get_curr_id()<<" received"<<std::endl;
    }

    

    if(get_idx()==0)
        timeshaft=get_curr_dts()*timebase;
    
    
    lock.unlock();
    seek_lock.unlock();
    action();
}