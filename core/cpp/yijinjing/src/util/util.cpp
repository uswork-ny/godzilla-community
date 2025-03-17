/**
 * This is source code modified under the Apache License 2.0.
 * Original Author: Keren Dong
 * Modifier: kx@godzilla.dev
 * Modification date: March 3, 2025
 */

#include <kungfu/yijinjing/util/os.h>

namespace kungfu {

    namespace yijinjing {

        namespace util {
            bool ends_with(const std::string& str, const std::string& suffix) {
                return str.size() >= suffix.size() &&
                    str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
            }
        }
    }
}
