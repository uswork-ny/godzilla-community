/**
 * This is source code modified under the Apache License 2.0.
 * Original Author: Keren Dong
 * Modifier: kx@godzilla.dev
 * Modification date: March 3, 2025
 */
#include <spdlog/spdlog.h>

#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/journal/journal.h>
#include <kungfu/yijinjing/journal/page.h>
#include <kungfu/yijinjing/util/util.h>
#include <kungfu/yijinjing/msg.h>

#define MAX_MD_PAGE_NUMBER 8
#define MAX_PAGE_NUMBER 50

namespace kungfu
{
    namespace yijinjing
    {
        namespace journal
        {

            journal::~journal()
            {
                if (current_page_.get() != nullptr)
                {
                    current_page_.reset();
                }
            }

            void journal::next()
            {
                assert(current_page_.get() != nullptr);
                if (frame_->msg_type() == msg::type::PageEnd)
                {
                    load_next_page();
                } else
                {
                    frame_->move_to_next();
                    page_frame_nb_++;
                }
            }

            void journal::seek_to_time(int64_t nanotime)
            {
                int page_id = page::find_page_id(location_, dest_id_, nanotime);
                load_page(page_id);
                SPDLOG_TRACE("{} in page [{}] [{} - {}]",
                             nanotime > 0 ? time::strftime(nanotime) : "beginning", page_id,
                             time::strftime(current_page_->begin_time(), "%F %T"), time::strftime(current_page_->end_time(), "%F %T"));
                while (current_page_->is_full() && current_page_->end_time() <= nanotime)
                {
                    load_next_page();
                }
                while (frame_->has_data() && frame_->gen_time() <= nanotime)
                {
                    next();
                }
            }

            void journal::load_page(int page_id)
            {
                if (current_page_.get() == nullptr or current_page_->get_page_id() != page_id)
                {
                    current_page_ = page::load(location_, dest_id_, page_id, is_writing_, lazy_);
                    frame_->set_address(current_page_->first_frame_address());
                    page_frame_nb_ = 0;
                }
                char* clear_journal = std::getenv("CLEAR_JOURNAL");
                if (clear_journal != nullptr and (is_writing_ || !lazy_))
                {
                    auto page_number = MAX_PAGE_NUMBER;
                    if (current_page_->get_location()->category == data::category::MD && current_page_->get_dest_id() == 0)
                    {
                        page_number = MAX_MD_PAGE_NUMBER;
                    }
                    if (page_id > page_number)
                    {
                        unlink(current_page_->get_page_path(current_page_->get_location(), current_page_->get_dest_id(), page_id - MAX_PAGE_NUMBER).c_str());
                    }
                }
            }

            void journal::load_next_page()
            {
                load_page(current_page_->get_page_id() + 1);
            }
        }
    }
}