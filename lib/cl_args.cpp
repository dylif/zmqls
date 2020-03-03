#include <string>

#include <zmqls/cl_args.hpp>

cxxopts::Options zmqls::cl_args::add_options()
{
        cxxopts::Options opt(name, desc);
        opt
                .positional_help("<file>")
        ;
        opt
                .allow_unrecognised_options()
                .add_options()
                ("help", this->help_desc, cxxopts::value(this->help))
                ("v,verbose", this->verbose_desc, cxxopts::value(this->verbose))
                ("t,threads", this->threads_desc, 
                        cxxopts::value<uint>(this->threads)
                        ->default_value(this->threads_def))
                ("file", this->file_desc, cxxopts::value<std::string>(this->file))
        ;

        return opt;
}

int zmqls::cl_args::parse(int argc, char **argv)
{
        // Add our options and parse for the input file
        auto opt = this->add_options();
        opt.parse_positional("file");

        // Try parsing and if we fail print the error
        try {
                opt.parse(argc, argv);
        } catch (const cxxopts::OptionParseException &e) {
                std::cerr << this->name << ": " << e.what() << std::endl;
                return EXIT_FAILURE;
        }

        // If we need show help show it, 
        // otherwise complain about not having an input file
        if (this->help) {
                std::cout << opt.help();
        } else if (this->file.empty()) {
                std::cerr << this->name << ": No input file" << std::endl;
                return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
}