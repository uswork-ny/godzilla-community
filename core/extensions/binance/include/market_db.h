/**
 * This is source code under the Apache License 2.0.
 * Original Author: kx@godzilla.dev
 * Original date: March 3, 2025
 */
#ifndef KF_EXT_XTC_MARKET_DB_H
#define KF_EXT_XTC_MARKET_DB_H

#include <SQLiteCpp/SQLiteCpp.h>
#include <spdlog/spdlog.h>
#include <kungfu/wingchun/common.h>

class MarketInfoDB {
public:
    MarketInfoDB(const std::string &file_name) : db_(file_name.c_str(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE)
    {
        create_table_if_not_exist();
    }

    void create_table_if_not_exist()
    {
        try
        {
            std::string sql = "CREATE TABLE IF NOT EXISTS market_info(\"symbol\" string NOT NULL,\"exchange_id\" string NOT NULL, \"instrument_type\" INTERGER, \"config\" string, \"update_time\" INTERGER, PRIMARY KEY(\"symbol\", \"exchange_id\", \"instrument_type\"));";
            db_.exec(sql);
        }
        catch (std::exception &e)
        {
            SPDLOG_TRACE(e.what());
        }
    }

    void add_market_info(const std::string &symbol, const std::string &exchange, kungfu::wingchun::InstrumentType instrument_type, const std::string &config, long insert_time)
    {
        try
        {
            SQLite::Statement insert(db_, "INSERT or replace INTO market_info VALUES(?, ?, ?, ?, ?)");
            insert.bind(1, symbol);
            insert.bind(2, exchange);
            insert.bind(3, static_cast<int>(instrument_type));
            insert.bind(4, config);
            insert.bind(5, insert_time);
            insert.exec();
        }
        catch (std::exception &e)
        {
            SPDLOG_ERROR(e.what());
        }
    }

    const std::string get_config(const std::string &symbol, const std::string &exchange, kungfu::wingchun::InstrumentType instrument_type)
    {
        std::string config;
        try
        {
            SQLite::Statement query(db_, "SELECT config FROM market_info WHERE symbol = ? AND exchange_id = ? AND instrument_type = ?");
            query.bind(1, symbol);
            query.bind(2, exchange);
            query.bind(3, static_cast<int>(instrument_type));
            if (query.executeStep())
            {
                const char* c = query.getColumn(0);
                config.append(c);
            }
        }
        catch (std::exception &e)
        {
            SPDLOG_ERROR(e.what());
        }
        return config;
    }

private:
    SQLite::Database db_;
};



#endif //KF_EXT_XTC_MARKET_DB_H
