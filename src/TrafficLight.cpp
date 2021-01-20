#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */
template <typename T>
T MessageQueue<T>::receive()
{    
    // perform queue modification under the lock
    std::unique_lock<std::mutex> uLock(_mutex);
    _condition.wait(uLock, [this] { return !_queue.empty(); }); // pass unique lock to condition variable
    // remove last vector element from queue
    T msg = std::move(_queue.back());
    _queue.pop_back();
    return msg; // will not be copied due to return value optimization (RVO) in C++
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // simulate some work
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // perform vector modification under the lock
    std::lock_guard<std::mutex> uLock(_mutex);
    // add vector to queue
    _queue.push_back(std::move(msg));
    _condition.notify_one(); // notify client after pushing new Vehicle into vector
}


/* Implementation of class "TrafficLight" */
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::kRed;
}

void TrafficLight::waitForGreen()
{
    while (true)
    {
        if (_queue.receive() == kGreen)
            return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    std::chrono::time_point<std::chrono::steady_clock> TimeSinceLastToggle;
    double TimeToNextToggle;
    TimeSinceLastToggle = std::chrono::steady_clock::now(); //Set init value
    TimeToNextToggle = rand() % 3 + 4; //Time to next toggle should be a random value in the range of 4-6
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        auto CurrentTime = std::chrono::steady_clock::now();
        std::chrono::duration<double> Elapsed_Time = CurrentTime - TimeSinceLastToggle;
        if (Elapsed_Time.count() >=  TimeToNextToggle)
        {
            if (_currentPhase == kRed)
                _currentPhase = kGreen;
            else
                _currentPhase = kRed;
            TimeSinceLastToggle = CurrentTime;
            TimeToNextToggle = rand() % 3 + 4;            
           _queue.send(std::move(_currentPhase));
        }
    }
}

