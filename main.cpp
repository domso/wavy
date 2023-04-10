#include <fstream>

#include "import/vcd_parser.h"
#include "frame_generator.h"

int main(const int argc, const char** args) {
    if (argc < 2) {
        std::cout << "Wrong usage" << std::endl;
        return -1;
    }

    std::string filename(args[1]);
    std::ifstream infile(filename);

    if (infile.good()) {
        wavy::import::vcd_parser<wavy::frame_generator> parser;

        wavy::frame_generator fg;
        parser.set_sink(fg);

        std::cout << "Read " << filename << std::endl;

        std::string line;
        while (std::getline(infile, line)) {
            parser.parse_line(line);
        }

        std::cout << "Read complete" << std::endl;

        fg.finalize();
        
        cmd::write_screen(fg);
    } else {
        std::cout << "Did not find " << filename << std::endl;
        return -1;
    }

    return 0;
}
