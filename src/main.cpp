#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/program_options.hpp>

#include "params.hpp"
#include "origin.hpp"
#include "image.hpp"

namespace {
    namespace po = boost::program_options;
    using irvm::Params;
    using irvm::IRVM_TOOL_COMMAND;

    po::options_description make_common_options();
    po::options_description make_io_options();
    void print_error(const std::string& command, const po::options_description& options, const std::string& error);
    
    Params parse_to_png(int argc, char* argv[]);
    Params parse_from_png(int argc, char* argv[]);
    Params parse_extract_png(int argc, char* argv[]);
    Params parse_from_ttf(int argc, char* argv[]);

    po::options_description make_common_options() {
        po::options_description options("Options");
        options.add_options()("help,h", "show this help message");
        return options;
    }


    po::options_description make_io_options() {
        po::options_description options("Input/output");
        options.add_options()
            ("input,i", po::value<std::string>()->required(), "input file")
            ("output,o", po::value<std::string>()->required(), "output file");

        return options;
    }


    void print_error(const std::string& command, const po::options_description& options, const std::string& error) {
        std::cerr << "Error in command '" << command << "':\n" << "  " << error << "\n\n" << options << '\n';
    }

    Params parse_to_png(int argc, char* argv[]) {
        po::options_description options("to-png options");
        options.add(make_common_options()).add(make_io_options());

        po::variables_map vm;

        try {
            po::store(po::command_line_parser(argc, argv).options(options).run(), vm);

            if(vm.count("help")) {
                std::cout << options << '\n';
                std::exit(0);
            }

            po::notify(vm);
        } catch(const po::error& e) {
            print_error("to-png", options, e.what());
            std::exit(1);
        }

        Params params;
        params.command = IRVM_TOOL_COMMAND::CONVERT_TO_PNG;
        params.input_file = vm["input"].as<std::string>();
        params.output_file = vm["output"].as<std::string>();

        return params;
    }

    Params parse_from_png(int argc, char* argv[]) {
        po::options_description options("from-png options");
        options.add(make_common_options()).add(make_io_options());

        po::variables_map vm;

        try {
            po::store(po::command_line_parser(argc, argv).options(options).run(), vm);

            if(vm.count("help")) {
                std::cout << options << '\n';
                std::exit(0);
            }

            po::notify(vm);
        } catch(const po::error& e) {
            print_error("from-png", options, e.what());
            std::exit(1);
        }

        Params params;
        params.command = IRVM_TOOL_COMMAND::CONVERT_FROM_PNG;
        params.input_file = vm["input"].as<std::string>();
        params.output_file = vm["output"].as<std::string>();

        return params;
    }

    Params parse_extract_png(int argc, char* argv[]) {
        po::options_description options("extract-png options");
        options.add(make_common_options()).add(make_io_options());

        po::variables_map vm;

        try {
            po::store(po::command_line_parser(argc, argv).options(options).run(), vm);

            if(vm.count("help")) {
                std::cout << options << '\n';
                std::exit(0);
            }

            po::notify(vm);
        } catch(const po::error& e) {
            print_error("extract-png", options, e.what());
            std::exit(1);
        }

        Params params;
        params.command = IRVM_TOOL_COMMAND::EXTRACT_PNG;
        params.input_file = vm["input"].as<std::string>();
        params.output_file = vm["output"].as<std::string>();

        return params;
    }

    Params parse_from_ttf(int argc, char* argv[]) {
        po::options_description options("from-ttf options");
        options.add(make_common_options()).add(make_io_options());

        po::variables_map vm;

        try {
            po::store(po::command_line_parser(argc, argv).options(options).run(), vm);

            if(vm.count("help")) {
                std::cout << options << '\n';
                std::exit(0);
            }

            po::notify(vm);
        } catch(const po::error& e) {
            print_error("from-ttf", options, e.what());
            std::exit(1);
        }

        Params params;
        params.command = IRVM_TOOL_COMMAND::CONVERT_FROM_TTF;
        params.input_file = vm["input"].as<std::string>();
        params.output_file = vm["output"].as<std::string>();

        return params;
    }
}

int main(int argc, char* argv[]) {
    using namespace irvm;

    if(argc < 2) {
        std::cerr << "No command provided.\n" << "Use 'irvm --help' for usage information.\n";
        return 1;
    }

    const std::string command = argv[1];

    try {
        Params params;

        if(command == "--help" || command == "-h") {
            std::cout
                << "Usage: irvm <command> [options]\n\n"
                << "Commands:\n"
                << "  to-png       Convert IRVM to PNG\n"
                << "  from-png     Convert PNG to IRVM\n"
                << "  extract-png  Extract PNG images from IRVM\n"
                << "  from-ttf     Convert TTF to IRVM\n\n"
                << "Use 'irvm <command> --help' for command-specific help.\n";

            return 0;
        }

        const int command_argc = argc - 1;
        char** command_argv = argv + 1;

        if(command == "to-png") {
            params = parse_to_png(command_argc, command_argv);
        } else if(command == "from-png") {
            params = parse_from_png(command_argc, command_argv);
        } else if(command == "extract-png") {
            params = parse_extract_png(command_argc, command_argv);
        } else if(command == "from-ttf") {
            params = parse_from_ttf(command_argc, command_argv);
        } else {
            std::cerr << "Unknown command: " << command << "\n" << "Use 'irvm --help' for usage information.\n";
            return 1;
        }

        switch (params.command) {
        case IRVM_TOOL_COMMAND::CONVERT_TO_PNG:
            break;

        case IRVM_TOOL_COMMAND::CONVERT_FROM_PNG: {
            OriginImage image = load_origin_image(params.input_file.c_str());
            std::cout << "Loaded PNG " << params.input_file << " (" << image.width << "x" << image.height << ")\n";

            std::vector<ColorCluster> clusters = cluster_colors(image, 24);
            for(const auto& cluster : clusters) {
                std::cout
                << "Cluster: RGB("
                << static_cast<int>(cluster.color.r) << ", "
                << static_cast<int>(cluster.color.g) << ", "
                << static_cast<int>(cluster.color.b)
                << ") Count: "
                << cluster.count
                << '\n';
            }

            break;
        }

        case IRVM_TOOL_COMMAND::EXTRACT_PNG:
            break;

        case IRVM_TOOL_COMMAND::CONVERT_FROM_TTF:
            break;
        }
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
