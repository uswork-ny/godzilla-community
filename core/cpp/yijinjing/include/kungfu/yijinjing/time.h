/**
 * This is source code modified under the Apache License 2.0.
 * Original Author: Keren Dong
 * Modifier: kx@godzilla.dev
 * Modification date: March 3, 2025
 */

#ifndef YIJINJING_TIMER_H
#define YIJINJING_TIMER_H

#include <string>

#define KUNGFU_DATETIME_FORMAT_DEFAULT "%F %T.%N"

namespace kungfu {

    namespace yijinjing {

        class time_unit {
        public:
            static const int64_t MILLISECONDS_PER_SECOND = 1000;
            static const int64_t MICROSECONDS_PER_MILLISECOND = 1000;
            static const int64_t NANOSECONDS_PER_MICROSECOND = 1000;

            static const int64_t MICROSECONDS_PER_SECOND = MICROSECONDS_PER_MILLISECOND * MILLISECONDS_PER_SECOND;
            static const int64_t NANOSECONDS_PER_MILLISECOND = NANOSECONDS_PER_MICROSECOND * MICROSECONDS_PER_MILLISECOND;
            static const int64_t NANOSECONDS_PER_SECOND = NANOSECONDS_PER_MILLISECOND * MILLISECONDS_PER_SECOND;

            static const int64_t SECONDS_PER_MINUTE = 60;
            static const int64_t MINUTES_PER_HOUR = 60;
            static const int64_t HOURS_PER_DAY = 24;
            static const int64_t SECONDS_PER_HOUR = SECONDS_PER_MINUTE * MINUTES_PER_HOUR;

            static const int64_t MILLISECONDS_PER_MINUTE = MILLISECONDS_PER_SECOND * SECONDS_PER_MINUTE;
            static const int64_t NANOSECONDS_PER_MINUTE = NANOSECONDS_PER_SECOND * SECONDS_PER_MINUTE;
            static const int64_t NANOSECONDS_PER_HOUR = NANOSECONDS_PER_SECOND * SECONDS_PER_HOUR;
            static const int64_t NANOSECONDS_PER_DAY = NANOSECONDS_PER_HOUR * HOURS_PER_DAY;
        };

        class time {
        public:

            /**
             * @return current nano time in int64_t (unix-timestamp * 1e9 + nano-part)
             */
            static int64_t now_in_nano();

            /**
             * parse string time to nano time
             * @param timestr string-formatted time
             * @param format ref: https://en.cppreference.com/w/cpp/io/manip/put_time + %N for nanoseconds {:09d}
             * @return nano time in int64_t
             */
            static int64_t strptime(const std::string &timestr, const std::string &format=KUNGFU_DATETIME_FORMAT_DEFAULT);

            /**
             * dump int64_t time to string with format
             * @param nanotime nano time in int64_t
             * @param format ref: https://en.cppreference.com/w/cpp/io/manip/put_time + %N for nanoseconds {:09d}
             * @return string-formatted time
             */
            static const std::string strftime(const int64_t nanotime, const std::string &format=KUNGFU_DATETIME_FORMAT_DEFAULT);

            static inline const std::string strfnow(const std::string &format=KUNGFU_DATETIME_FORMAT_DEFAULT)
            {
                return strftime(now_in_nano(), format);
            }

            static inline int64_t next_minute_nano(int64_t nanotime)
            {
                return nanotime - nanotime % time_unit::NANOSECONDS_PER_MINUTE + time_unit::NANOSECONDS_PER_MINUTE;
            }

            static inline int64_t next_day_nano(int64_t nanotime)
            {
                int64_t day = nanotime - nanotime % time_unit::NANOSECONDS_PER_DAY
                        + 7 * time_unit::NANOSECONDS_PER_HOUR + 30 * time_unit::NANOSECONDS_PER_MINUTE;
                if (day < now_in_nano())
                {
                    day += time_unit::NANOSECONDS_PER_DAY;
                }
                return day;
            }

        private:
            time();
            static const time& get_instance();

            int64_t start_time_since_epoch_;
            int64_t start_time_steady_;
        };
    }
}

#endif //YIJINJING_TIMER_H