/*
 * Copyright (c) 2008 - 2012, Roberto Bruttomesso
 * Copyright (c) 2022, Antti Hyvarinen <antti.hyvarinen@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <string.h>
#include <ostream>
#include <iostream>
#include <sys/time.h>
#include <chrono>
#include <sys/resource.h>
#include <cassert>

namespace opensmt {
// A c++ wrapper for struct timeval
    class BTime {
        time_t tv_sec;
        suseconds_t tv_usec;
    public:
        BTime() : tv_sec(0), tv_usec(0) {}

        BTime(const struct timeval & tv) : tv_sec(tv.tv_sec), tv_usec(tv.tv_usec) {}

        BTime(const BTime & from) : tv_sec(from.tv_sec), tv_usec(from.tv_usec) {}

        void operator-=(const BTime & subst) {
            tv_sec -= subst.tv_sec;
            tv_usec -= subst.tv_usec;
        }

        BTime operator-(const BTime & subst) {
            BTime out;
            out.tv_sec = tv_sec - subst.tv_sec;
            out.tv_usec = tv_usec - subst.tv_usec;
            return out;
        }

        void operator=(const BTime & from) {
            tv_sec = from.tv_sec;
            tv_usec = from.tv_usec;
        }

        BTime & operator+=(const BTime & other) {
            tv_sec += other.tv_sec;
            tv_usec += other.tv_usec;
            return *this;
        }

        double getTime() {
            return tv_sec + tv_usec / (double) 1000000;
        }
    };

    class OSMTTimeVal {
        BTime usrtime;
        BTime systime;
    public:
        OSMTTimeVal() {}

        OSMTTimeVal(const BTime & usrtime, const BTime & systime) : usrtime(usrtime), systime(systime) {}

        OSMTTimeVal(const struct rusage & res_usage) : usrtime(res_usage.ru_utime), systime(res_usage.ru_stime) {}

        void operator-=(const OSMTTimeVal & subst) {
            usrtime -= subst.usrtime;
            systime -= subst.systime;
        }

        OSMTTimeVal operator-(const OSMTTimeVal & subst) {
            OSMTTimeVal out;
            out.usrtime = usrtime - subst.usrtime;
            out.systime = systime - subst.systime;
            return out;
        }

        OSMTTimeVal & operator+=(const OSMTTimeVal & from) {
            usrtime += from.usrtime;
            systime += from.systime;
            return *this;
        }

        double getTime() {
            return usrtime.getTime() + systime.getTime();
        }
    };

    class StopWatch {
    protected:
        struct rusage tmp_rusage;
        OSMTTimeVal time_start;
        OSMTTimeVal & timer;
    public:
        StopWatch(OSMTTimeVal & timer)
                : timer(timer) {
            if (getrusage(RUSAGE_SELF, &tmp_rusage) == 0) {
                time_start = OSMTTimeVal(tmp_rusage);
            } else
                assert(false);
        }

        ~StopWatch() {
            if (getrusage(RUSAGE_SELF, &tmp_rusage) == 0) {
                timer += OSMTTimeVal(tmp_rusage) - time_start;
            } else
                assert(false);
        }
    };

// A c++ wrapper for manual time checking
    class StoppableWatch {

    public:
        StoppableWatch(const StoppableWatch & other) = default;

        StoppableWatch(StoppableWatch && other) = default;

        virtual ~StoppableWatch() = default;

        StoppableWatch & operator=(const StoppableWatch & other) = default;

        StoppableWatch & operator=(StoppableWatch && other) = default;

        inline StoppableWatch(bool start = false) :
                started_(false), paused_(false),
                reference_(std::chrono::steady_clock::now()),
                accumulated_(std::chrono::duration<long double>(0)) {
            if (start)
                this->start();
        }

        inline void start() {
            if (not started_) {
                started_ = true;
                paused_ = false;
                accumulated_ = std::chrono::duration<long double>(0);
                reference_ = std::chrono::steady_clock::now();
            } else if (paused_) {
                reference_ = std::chrono::steady_clock::now();
                paused_ = false;
            }
        }

        inline void stop() {
            if (started_ && not paused_) {
                std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
                accumulated_ = accumulated_ +
                               std::chrono::duration_cast<std::chrono::duration<long double> >(now - reference_);
                paused_ = true;
            }
        }

        inline void reset() {
            if (started_) {
                started_ = false;
                paused_ = false;
                reference_ = std::chrono::steady_clock::now();
                accumulated_ = std::chrono::duration<long double>(0);
            }
        }

        inline std::uint_fast16_t elapsed_time_milliseconds() {
            return count_elapsed<std::chrono::milliseconds>();
        }

        inline std::uint_fast8_t elapsed_time_second() {
            return count_elapsed<std::chrono::seconds>();
        }

        template<class duration_t>
        typename duration_t::rep count_elapsed() const {
            if (started_) {
                if (paused_) {
                    return std::chrono::duration_cast<duration_t>(accumulated_).count();
                } else {
                    return std::chrono::duration_cast<duration_t>(
                            accumulated_ + (std::chrono::steady_clock::now() - reference_)).count();
                }
            } else {
                return duration_t(0).count();
            }
        }

    private:
        bool started_;
        bool paused_;
        std::chrono::steady_clock::time_point reference_;
        std::chrono::duration<long double> accumulated_;
    };
}