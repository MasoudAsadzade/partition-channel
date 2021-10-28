#pragma once

#include <vector>
#include <mutex>
#include <condition_variable>
#include "atomic"
#include "deque"
#include <map>

class Channel {
    using tp_t = std::chrono::time_point<std::chrono::system_clock>;
    using td_t = const std::chrono::duration<double>;
    mutable std::mutex              mutex;
    mutable std::mutex              stop_mutex;
    mutable std::condition_variable cv_;
    mutable std::condition_variable cv;

    std::deque<std::string>         queries;
    std::atomic_bool                requestStop;
    std::atomic_bool                stop;
    bool                            isClauseShareMode;
    bool                            isClauseInjection;
    int                             clauseLearnInterval;
    std::string                     workingNode;
    std::atomic_bool                isSpuriousWakeup_ForPush;
    std::atomic_bool                isSpuriousWakeup_ForPull;

    std::map<std::string, std::vector<std::pair<std::string ,int>>> clauses;
//    std::priority_queue<int,std::vector<int>,std::less<int> > pq;
//    bool shouldStop()                    { return std::atomic_flag_test_and_set_explicit(&requestStop, std::memory_order_acquire); }
//    v.oid clearShouldStop()                  { requestStop.clear(std::memory_order_release); }
public:

    void assign(std::vector<std::pair<std::string ,int>> toPublishTerms)
    {
        clauses[workingNode].insert(std::end(clauses[workingNode]), std::begin(toPublishTerms), std::end(toPublishTerms));
//        clauses[workingNode].assign(toPublishTerms.begin(), toPublishTerms.end());
    }
    Channel() : requestStop(false), stop(false), isClauseShareMode(false), isClauseInjection(false),
                clauseLearnInterval(1000), isSpuriousWakeup_ForPush(false), isSpuriousWakeup_ForPull(false) {
    }

    std::map<std::string, std::vector<std::pair<std::string ,int>>> & getClauseMap() {return clauses;};
    std::mutex & getMutex()                 { return mutex; }
    std::mutex & getStopMutex()             { return stop_mutex; }
    size_t size() const                     { return clauses.size(); }
    auto cbegin() const                     { return clauses.cbegin(); }
    auto cend() const                       { return clauses.cend(); }
    auto begin() const                      { return clauses.begin(); }
    auto end() const                        { return clauses.end(); }
    void notify_one()                       { cv.notify_one(); }
    void notify_one_()                      { cv_.notify_one(); }
    void notify_all()                       { cv.notify_all(); }
    void clear()                            { clauses.clear(); }
    bool empty() const                      { return clauses.empty(); }
    bool shouldTerminate() const            { return stop; }
    void setTerminate()                     { stop = true; }
    void clearTerminate()                     { stop = false; }
    bool shouldStop() const                 { return requestStop; }
    void setShouldStop()                    { requestStop = true; }
    void clearShouldStop()                  { requestStop = false; }
    bool shallClauseShareMode() const       { return isClauseShareMode; }
    void setClauseShareMode()               { isClauseShareMode = true; }
    void clearClauseShareMode()             { isClauseShareMode = false; }
    void push_back_query(const std::string& str)  { queries.push_back(str);}
    void pop_front_query()                  { queries.pop_front();}
    size_t size_query() const               { return queries.size(); }
    void setWorkingNode(std::string& n)     { workingNode = n; }
    std::string getWorkingNode() const      { return workingNode; }
    bool isEmpty_query() const              { return queries.empty(); }
    void clear_query()                      { queries.clear() ; }
    auto cbegin_query() const               { return queries.cbegin(); }
    auto cend_query() const                 { return queries.cend(); }
    auto get_queris() const                 { return queries; }
    bool isInjection() const                { return isClauseInjection; }
    void setClauseInjection()               { isClauseInjection = true; }
    void clearClauseInjection()             { isClauseInjection = false; }
    bool shouldSpuriousWakeup_ForPush()  const  { return isSpuriousWakeup_ForPush;   }
    void setSpuriousWakeup_ForPush()            { isSpuriousWakeup_ForPush = true;  }
    void clearSpuriousWakeup_ForPush()          { isSpuriousWakeup_ForPush = false; }
    bool shouldSpuriousWakeup_ForPull()  const  { return isSpuriousWakeup_ForPull;   }
    void setSpuriousWakeup_ForPull()            { isSpuriousWakeup_ForPull = true;  }
    void clearSpuriousWakeup_ForPull()          { isSpuriousWakeup_ForPull = false; }
    void setClauseLearnInterval(int wait)        { clauseLearnInterval = wait; }
    int getClauseLearnInterval() const          { return clauseLearnInterval; }
    void clearChannel()          { clear(); clear_query(); clearShouldStop(); clearTerminate(); clearClauseShareMode(); clearClauseInjection(); }

    bool waitUntil(std::unique_lock<std::mutex> & lock, const tp_t& timepoint)
    {
        return cv.wait_until(lock, timepoint, [&] { return not isEmpty_query(); });
    }
    bool waitFor_push(std::unique_lock<std::mutex> & lock, const td_t& timeout_duration)
    {
        return cv.wait_for(lock, timeout_duration, [&] { return shouldTerminate(); });
    }
    bool waitFor_pull(std::unique_lock<std::mutex> & lock, const td_t& timeout_duration)
    {
        return cv.wait_for(lock, timeout_duration, [&] { return shouldTerminate(); });
    }
    bool waitFor(std::unique_lock<std::mutex> & lock, const td_t& timeout_duration)
    {
        return cv.wait_for(lock, timeout_duration, [&] { return shouldTerminate(); });
    }
    void waitForQueryOrTemination(std::unique_lock<std::mutex> & lock)
    {
        cv.wait(lock, [&] { return (shouldTerminate() or not isEmpty_query());});
    }
};




