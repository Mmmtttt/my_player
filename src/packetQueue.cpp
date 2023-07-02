#include "packetQueue.h"

int video_packetQueue::packet_queue_push(std::shared_ptr<myAVPacket> pkt_ptr)
{
    //SDL_LockMutex(q->mutex);
    std::unique_lock<std::mutex> lock(Mutex);
    size += pkt_ptr->size;
    pkts_ptr.push_back(std::move(pkt_ptr));
    cond.notify_one();
    lock.unlock();
    return 0;
}

int video_packetQueue::packet_queue_pop(std::shared_ptr<myAVPacket>& pkt_ptr, int block)
{
    int ret;

    //SDL_LockMutex(q->mutex);
std::unique_lock<std::mutex> lock(Mutex);
    while (1)
    {
        if (pkts_ptr.size())             // 队列非空，取一个出来
        {//std::cout<<pkts_ptr.size()<<std::endl;
            size -= pkts_ptr.front()->size;
            pkt_ptr=std::move(pkts_ptr.front());//std::cout<<pkts_ptr.front().mypkt.buf<<std::endl;
        //std::cout<<"de 1";
            pkts_ptr.pop_front();//std::cout<<q->pkts.size()<<std::endl;
        //std::cout<<"de 2";

            ret = 1;//std::cout<<pkt.size<<std::endl;
            break;
        }
        // else if (s_input_finished)  // 队列已空，文件已处理完
        // {
        //     ret = 0;
        //     break;
        // }
        else if (!block)            // 队列空且阻塞标志无效，则立即退出
        {
            ret = 0;
            break;
        }
        else                        // 队列空且阻塞标志有效，则等待
        {
            //SDL_CondWait(q->cond, q->mutex);
            cond.wait(lock);
        }
    }
    //SDL_UnlockMutex(q->mutex);
    lock.unlock();
    return ret;
}