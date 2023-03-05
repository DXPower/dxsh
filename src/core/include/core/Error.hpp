#pragma once

#include <string>
#include <vector>

namespace dxsh {
    namespace core {
        struct Error {
            int line;
            std::string message;
        };

        using ErrorContext = std::vector<Error>;

        
    }
}