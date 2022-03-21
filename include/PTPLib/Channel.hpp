/*
 * Copyright (c) 2022, Antti Hyvarinen <antti.hyvarinen@gmail.com>
 * Copyright (c) 2022, Seyedmasoud Asadzadeh <seyedmasoud.asadzadeh@usi.ch>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef PTPLIB_CHANNEL_HPP
#define PTPLIB_CHANNEL_HPP

#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <map>
#include <algorithm>
#include <chrono>
#include "Header.hpp"
#include "ThreadSafeContainer.hpp"
#include <queue>

class Channel {

    using td_t = const std::chrono::duration<double>;
    mutable std::mutex mutex;
    mutable std::condition_variable cv;
    std::queue<std::pair<PTPLib::safe_ptr<PTPLib::net::Header>, std::string>> queries;
    PTPLib::net::Header currentHeader;
    std::atomic_bool requestStop;
    std::atomic_bool terminate;

    bool clauseShareMode;
    bool isFirstTime;
    bool injectClause;
    int clauseLearnDuration;
    std::string currentSolverAddress;
    bool apiMode;

    std::map<std::string, std::vector<std::pair<std::string, int>>> m_learned_clauses;
    std::map<std::string, std::vector<std::pair<std::string, int>>> m_pulled_clauses;
public:
    Channel() :
        requestStop         (false),
        terminate           (false),
        clauseShareMode     (false),
        isFirstTime         (false),
        injectClause        (false),
        clauseLearnDuration (4000),
        apiMode             (false)
        {}

    void insert_learned_clause(std::vector<std::pair<std::string, int>> & toPublishTerms)
    {
        m_learned_clauses[currentSolverAddress].insert(std::end(m_learned_clauses[currentSolverAddress]),
                                             std::begin(toPublishTerms), std::end(toPublishTerms));
    }

    void insert_pulled_clause(std::vector<std::pair<std::string, int>> & toPublishTerms) {
        m_pulled_clauses[currentSolverAddress].insert(std::end(m_pulled_clauses[currentSolverAddress]),
                                                      std::begin(toPublishTerms), std::end(toPublishTerms));
    }
    void pop_front_query() { queries.pop();}

    size_t size_query() const { return queries.size(); }

    bool isEmpty_query() const { return queries.empty(); }

    auto get_queris() const { return queries; }

    auto get_FrontQuery() const { return queries.front(); }

    void push_back_query(std::pair<PTPLib::safe_ptr< PTPLib::net::Header>, std::string> & hd) { queries.push(std::move(hd)); }

    std::map<std::string, std::vector<std::pair<std::string, int>>> get_learned_clauses() const { return std::move(m_learned_clauses); };

    std::map<std::string, std::vector<std::pair<std::string, int>>> & get_pulled_clauses()   { return m_pulled_clauses; };

    std::mutex & getMutex() { return mutex; }

    size_t size() const { return m_learned_clauses.size(); }

    auto cbegin() const { return m_learned_clauses.cbegin(); }

    auto cend() const { return m_learned_clauses.cend(); }

    auto begin() const { return m_learned_clauses.begin(); }

    auto end() const { return m_learned_clauses.end(); }

    void notify_one() { cv.notify_one(); }

    void notify_all() { cv.notify_all(); }

    void clear_learned_clauses() { m_learned_clauses.clear(); }

    void clear_pulled_clauses() { m_pulled_clauses.clear(); }

    bool empty() const { return m_learned_clauses.empty(); }

    bool shouldTerminate() const { return terminate; }

    void setTerminate() { terminate = true; }

    void clearTerminate() { terminate = false; }

    bool shouldStop() const { return requestStop; }

    void setShouldStop() { requestStop = true; }

    void clearShouldStop() { requestStop = false; }

    bool isClauseShareMode() const { return clauseShareMode; }

    void setClauseShareMode() { clauseShareMode = isFirstTime = true; }

    void clearClauseShareMode() { clauseShareMode = false; }

    bool shouldInjectClause() const { return injectClause; }

    void setInjectClause() { injectClause = true; }

    void clearInjectClause() { injectClause = false; }

    bool getFirstTimeLearnClause() const { return isFirstTime; }

    void clearFirstTimeLearnClause() { isFirstTime = false; }

    void setClauseLearnDuration(int cld) { clauseLearnDuration = cld; }

    int getClauseLearnDuration() const { return clauseLearnDuration; }

    void move_Header(PTPLib::net::Header  & hd) { currentHeader.moveIn(hd); }

    PTPLib::net::Header & getHeader()  { return currentHeader; }

    bool isApiMode() const { return apiMode; }

    void setApiMode() { apiMode = true; }

    void clearApiMode() { apiMode = false; }

    void setCurrentSolverAddress(std::string csa) { currentSolverAddress = csa; }

    std::string getCurrentSolverAddress() const { return currentSolverAddress; }

    bool waitFor(std::unique_lock<std::mutex> & lock, const td_t & timeout_duration)
    {
        return cv.wait_for(lock, timeout_duration, [&] { return shouldTerminate(); });
    }

    void waitQueryOrTermination(std::unique_lock<std::mutex> & lock)
    {
        cv.wait(lock, [&] { return (shouldTerminate() or not isEmpty_query()); });
    }

    void resetChannel() {
        clear_pulled_clauses();
        clear_learned_clauses();
        clearShouldStop();
        clearTerminate();
        clearClauseShareMode();
        clearInjectClause();
        setApiMode();
    }

};

#endif PTPLIB_CHANNEL_HPP