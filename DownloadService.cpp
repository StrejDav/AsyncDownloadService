#include "DownloadService.hpp"
#include <string>
#include <iostream>
#include <curl/curl.h>

size_t writeData(void* ptr, size_t size, size_t nmemb, FILE* stream);

DownloadService::~DownloadService() {
    if (_serviceRunning) {
        stopService();
    }
}

void DownloadService::startService(unsigned int nWorkerThreads) {
    if (!_serviceRunning) {
        _nWorkerThreads = nWorkerThreads;
        _serviceRunning = true;
        for (int i = 0; i < _nWorkerThreads; i++) {
            _threadPool.emplace_back(&DownloadService::_doWork, this);
        }
    }
}

void DownloadService::stopService() {
    if (_serviceRunning) {
        _serviceRunning = false;
        _cv.notify_all();
        for (auto& currThread: _threadPool) {
            currThread.join();
        }
    }
}

void DownloadService::pushToDownloadQueue(const std::string& url) {
    {
        std::unique_lock<std::mutex> lck(_mtx);
        _downloadQueue.push(url);
    }
    _cv.notify_one();
}

void DownloadService::_doWork() {
    std::string downloadUrl;

    while (_serviceRunning) {
        {
            std::unique_lock<std::mutex> lck(_mtx);
            _cv.wait(lck, [this]{ return (!_downloadQueue.empty() || !_serviceRunning); });
            if (!_serviceRunning) {
                break;
            }
            downloadUrl = _downloadQueue.front();
            _downloadQueue.pop();
        }

        _downloadFile(downloadUrl);
    }
}

void DownloadService::_downloadFile(const std::string& url) {
    std::string filename = _getFilenameFromUrl(url);
    // std::string filename = "asd";
    FILE* fp = fopen(filename.c_str(), "wb");

    CURL* curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    CURLcode res = curl_easy_perform(curl);

    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_easy_cleanup(curl);
    fclose(fp);

    if (res != CURLE_OK) {
        std::remove(filename.c_str());
        std::cerr << "Download of file \'" << filename.c_str() << "\' failed: " << curl_easy_strerror(res) << std::endl << "> ";
    }
    else if (http_code >= 400) {
        std::remove(filename.c_str());
        std::cerr << "Download of file \'" << filename.c_str() << "\' failed. HTTP code: " << http_code << std::endl << "> ";
    }
}

std::string DownloadService::_getFilenameFromUrl(const std::string& url) {
    size_t pos = url.find_last_of('/');
    if (pos == std::string::npos || pos == url.size() - 1)
        return "file" + std::to_string(_nFile++); // fallback if no filename

    return url.substr(pos + 1);
}

size_t writeData(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    return fwrite(ptr, size, nmemb, stream);
}
