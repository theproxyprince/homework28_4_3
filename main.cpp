#include <iostream>
#include <thread>
#include <mutex>
#include <ctime>
#include <queue>

class Restaurant {
    std::queue<std::string> newOrderQueue;
    std::queue<std::string> readyOrderQueue;
    std::mutex newOrderMutex;
    std::mutex readyOrderMutex;
    std::mutex print;
    std::string orderToPrepare;
    int readyOrderQuantity = 0;
    std::array<std::string, 5> order {"pizza", "soup", "steak", "salad", "sushi"};

public:
    void Waiter() {
        do {
            std::srand(std::time(nullptr));
            int index = std::rand() % 5;
            int waitTime = std::rand() % 6 + 5;

            std::unique_lock<std::mutex> ulNew(newOrderMutex);
            newOrderQueue.push(order.at(index));

            std::unique_lock<std::mutex> ulPr(print);
            std::cout << "New order from visitor: " << order.at(index) << std::endl;
            ulPr.unlock();
            ulNew.unlock();

            std::this_thread::sleep_for(std::chrono::seconds (waitTime));
        } while (readyOrderQuantity < 10);

    }
    void Prepare(){
        std::srand(std::time(nullptr));
        int waitTime = std::rand() % 11 + 5;

        std::unique_lock<std::mutex> ulPr(print);
        std::cout << "The kitchen accepted the order: " << orderToPrepare;
        std::cout << " Time to prepare - " << waitTime << " sec" << std::endl;
        ulPr.unlock();

        std::this_thread::sleep_for(std::chrono::seconds (waitTime));
        std::unique_lock<std::mutex> ulReady(readyOrderMutex);
        readyOrderQueue.emplace(orderToPrepare);
        ulReady.unlock();

        ulPr.lock();
        std::cout << "The order: " << orderToPrepare << " is ready!" << std::endl;
        ulPr.unlock();
    }
    void Cook() {
        do {
            bool newOrder = false;
            orderToPrepare.erase();

            std::unique_lock<std::mutex> ulNew(newOrderMutex);
            if (!newOrderQueue.empty()) {
                orderToPrepare = newOrderQueue.front();
                newOrderQueue.pop();
                newOrder = true;
            }
            ulNew.unlock();

            if (newOrder) {
                std::thread prepare(&Restaurant::Prepare, this);
                prepare.join();
            }

        } while (readyOrderQuantity < 10);
    }

    void Courier() {
        do {
            std::string orderToDelivery;
            std::unique_lock<std::mutex> ulReady(readyOrderMutex);
            if (!readyOrderQueue.empty()) {
                orderToDelivery = readyOrderQueue.front();
                readyOrderQueue.pop();

                std::unique_lock<std::mutex> ulPr(print);
                std::cout << "The order number " << readyOrderQuantity + 1 << " : " << orderToDelivery << " has been moved to delivery!" << std::endl;
                ulPr.unlock();
                std::this_thread::sleep_for(std::chrono::seconds (30));

                ulPr.lock();
                std::cout << "The order number " << readyOrderQuantity + 1<< " : " << orderToDelivery << " has been delivered!" << std::endl;
                readyOrderQuantity++;
                ulPr.unlock();
            }
            ulReady.unlock();
        } while(readyOrderQuantity < 10);

    }

    int GetReadyOrderQuantity() const {
        return readyOrderQuantity;
    }

    std::mutex& GetPrinterMutex() {
        std::mutex& mutex = print;
        return mutex;
    }
};


int main() {
    Restaurant restaurant;
    std::thread waiter(&Restaurant::Waiter, std::ref(restaurant));
    std::thread cook(&Restaurant::Cook, std::ref(restaurant));
    std::thread courier(&Restaurant::Courier, std::ref(restaurant));
    waiter.detach();
    cook.detach();
    courier.detach();

    while (restaurant.GetReadyOrderQuantity() < 10);
    std::unique_lock<std::mutex>ulPrinter(restaurant.GetPrinterMutex());
    std::cout << "Exiting the program..." << std::endl;
    return 0;
}