#pragma once

#include <condition_variable>
#include <thread>
#include <queue>
#include <atomic>
#include <vector>

class DownloadService {
public:
    DownloadService() = default;
    ~DownloadService();
    void startService(unsigned int nWorkerThreads);
    void stopService();
    void pushToDownloadQueue(const std::string& url);

private:
    unsigned int _nWorkerThreads;
    std::queue<std::string> _downloadQueue;
    std::condition_variable _cv;
    std::mutex _mtx;
    std::atomic<bool> _serviceRunning = false;
    std::vector<std::thread> _threadPool;
    static inline std::atomic<unsigned int> _nFile = 0; // Used in case there is no filename

    void _doWork();
    void _downloadFile(const std::string& url);
    std::string _getFilenameFromUrl(const std::string& url);
};
