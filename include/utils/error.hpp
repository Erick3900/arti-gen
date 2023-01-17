#pragma once

#include <string>

#include <tl/expected.hpp>

namespace arti {

    template <typename ErrorEnum, typename ErrorInfo>
    struct error_t {
        ErrorEnum error;
        ErrorInfo info;
    };

    template <typename ExpectedType, typename ErrorType>
    using expected = tl::expected<ExpectedType, error_t<ErrorType, std::string>>;

}