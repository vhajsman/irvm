#ifndef __IRVM_PARAMS_HPP__
#define __IRVM_PARAMS_HPP__

#include <string>

namespace irvm {
    enum class IRVM_TOOL_COMMAND {
        CONVERT_TO_PNG,
        EXTRACT_PNG,
        CONVERT_FROM_PNG,
        CONVERT_FROM_TTF
    };

    struct Params {
        IRVM_TOOL_COMMAND command;

        std::string input_file;
        std::string output_file;
    };
}

#endif // __IRVM_PARAMS_HPP__
