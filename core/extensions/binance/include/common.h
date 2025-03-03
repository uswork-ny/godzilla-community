/**
 * This is source code under the Apache License 2.0.
 * Original Author: kx@godzilla.dev
 * Original date: March 3, 2025
 */

#ifndef GODZILLA_BINANCE_EXT_COMMON_H
#define GODZILLA_BINANCE_EXT_COMMON_H

#include <nlohmann/json.hpp>

namespace kungfu
{
    namespace wingchun
    {
        namespace binance
        {
            struct Configuration
            {
                std::string user_id;
                std::string access_key;
                std::string secret_key;
                std::string spot_rest_host;  // 现货rest域名
                int spot_rest_port;          // 现货rest端口
                std::string spot_wss_host;   // 现货websocket域名
                int spot_wss_port;           // 现货websocket端口
                std::string ubase_rest_host; // u本位合约rest域名
                int ubase_rest_port;         // u本位合约rest端口
                std::string ubase_wss_host;  // u本位合约websocket域名
                int ubase_wss_port;          // u本位合约wesocket端口
                std::string cbase_rest_host; // 币本位合约rest域名
                int cbase_rest_port;         // 币本位合约rest端口
                std::string cbase_wss_host;  // 币本位合约websocket域名
                int cbase_wss_port;          // 币本位合约websocket端口
            };

            inline void from_json(const nlohmann::json &j, Configuration &c)
            {
                j.at("user_id").get_to(c.user_id);
                j.at("access_key").get_to(c.access_key);
                j.at("secret_key").get_to(c.secret_key);
                c.spot_rest_host = "api.binance.com";
                c.spot_rest_port = 443;
                c.spot_wss_host = "stream.binance.com";
                c.spot_wss_port = 443;
                c.ubase_rest_host = "fapi.binance.com";
                c.ubase_rest_port = 443;
                c.ubase_wss_host = "fstream.binance.com";
                c.ubase_wss_port = 443;
                c.cbase_rest_host = "dapi.binance.com";
                c.cbase_rest_port = 443;
                c.cbase_wss_host = "dstream.binance.com";
                c.cbase_wss_port = 443;
            }
        }
    }
}
#endif //GODZILLA_BINANCE_EXT_COMMON_H
