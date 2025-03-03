/**
 * This is source code under the Apache License 2.0.
 * Original Author: kx@godzilla.dev
 * Original date: March 3, 2025
 */

#ifndef GODZILLA_BINANCE_EXT_ORDER_MAPPER_H
#define GODZILLA_BINANCE_EXT_ORDER_MAPPER_H


#include <iostream>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <kungfu/wingchun/msg.h>

namespace bmi = boost::multi_index;


//Data to insert in shared memory
struct order_map_record
{
   std::size_t orderId;
   std::string exOrderId;
   std::size_t source;
   std::size_t lastUpdate;
   kungfu::wingchun::msg::data::Order order;

   order_map_record( std::size_t orderId_
           , std::string exOrderId_
           , std::size_t source_
           , std::size_t lastUpdate_
           , kungfu::wingchun::msg::data::Order order_)
      : orderId(orderId_), exOrderId(exOrderId_), source(source_), lastUpdate(lastUpdate_), order(order_)
   {}
};

//Tags
struct orderId{};
struct exOrderId{};
struct lastUpdate{};

typedef bmi::multi_index_container<
  order_map_record,
  bmi::indexed_by<
    bmi::ordered_unique
      <bmi::tag<orderId>,  BOOST_MULTI_INDEX_MEMBER(order_map_record,std::size_t,orderId)>,
    bmi::ordered_unique
      <bmi::tag<exOrderId>, BOOST_MULTI_INDEX_MEMBER(order_map_record,std::string,exOrderId)>,
    bmi::ordered_non_unique
      <bmi::tag<lastUpdate>, BOOST_MULTI_INDEX_MEMBER(order_map_record,std::size_t,lastUpdate)> >
> OrderSet;

struct change_order {
  change_order(kungfu::wingchun::msg::data::Order order):order_(order){}

  void operator()(order_map_record &s) {
    s.order = order_;
  }

private:
  kungfu::wingchun::msg::data::Order order_;
};

#endif  // GODZILLA_BINANCE_EXT_ORDER_MAPPER_H