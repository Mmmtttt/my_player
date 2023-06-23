#include "packetQueue.h"

int packetQueue::packet_queue_push(AVPacket *pkt)
{
    std::unique_lock<std::mutex> lock(Mutex);
    //SDL_LockMutex(q->mutex);
    pkts.push_back(*pkt);
    size += pkt->size;
    
    // 发个条件变量的信号：重启等待q->cond条件变量的一个线程
    cond.notify_one();
    lock.unlock();
    return 0;
}

int packetQueue::packet_queue_pop(AVPacket *pkt, int block)
{
    int ret;

    std::unique_lock<std::mutex> lock(Mutex);

    while (1)
    {
        if (pkts.size())             // 队列非空，取一个出来
        {
            *pkt=pkts.front();
            pkts.pop_front();
            ret = 1;
            break;
        }
        else if (!block)            // 队列空且阻塞标志无效，则立即退出
        {
            ret = 0;
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