#include <iostream>
#include <thread>
#include<mutex>
#include <shared_mutex>
#include <vector>

class SharedResource {
public:
    void write(int value) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        data_ = value;
        std::cout << "Writing " << value << std::endl;
    }

    int read() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        std::cout << "Reading " << data_ << std::endl;
        return data_;
    }

private:
    mutable std::shared_mutex mutex_;
    int data_ = 0;
};

int main() {
    SharedResource resource;

    // 创建多个读者线程
    std::vector<std::thread> readers;
    for (int i = 0; i < 5; ++i) {
        readers.emplace_back([&resource, i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100 * i));
            resource.read();
        });
    }

    // 创建一个写者线程
    std::thread writer([&resource]() {
        for (int i = 1; i <= 5; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
            resource.write(i);
        }
    });

    // 等待所有线程完成
    for (auto& reader : readers) {
        reader.join();
    }
    writer.join();

    return 0;
}

