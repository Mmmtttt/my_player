#include "packetQueue.h"

int packetQueue::packet_queue_push(std::shared_ptr<myAVPacket> pkt_ptr)
{
    std::unique_lock<std::mutex> lock(Mutex);

    size += pkt_ptr->size;
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
            pkt_ptr=pkts_ptr[curr_decode_pos];
            curr_decode_pos++;
            ret = 1;
            break;
        }
        
        else                        // 队列空且阻塞标志有效，则等待
        {
            cond.wait(lock);
        }
    }
    lock.unlock();
    return ret;
}