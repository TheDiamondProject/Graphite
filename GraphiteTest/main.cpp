// Copyright (c) 2020 Tom Hancocks
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "libGraphite/rsrc/file.hpp"
#include "libGraphite/data/writer.hpp"
#include <iostream>

int main(int argc, char const *argv[])
{
    auto rf = std::make_shared<graphite::rsrc::file>();
    
    auto english = std::make_shared<graphite::data::writer>();
    english->write_cstr("Hello, World!");
    rf->add_resource("test", 128, "test resource", english->data(), {
        std::make_pair("lang", "en")
    });

    auto french = std::make_shared<graphite::data::writer>();
    french->write_cstr("Bonjour, Monde!");
    rf->add_resource("test", 128, "test resource", french->data(), {
        std::make_pair("lang", "fr")
    });

    // The resource file should be assembled at this point and just needs writing to disk.
    rf->write("test.cdat", graphite::rsrc::file::format::extended);


    auto in_rf = std::make_shared<graphite::rsrc::file>("test.cdat");
    for (const auto& type : in_rf->types()) {
        std::cout << "reading type: " << type->code() << type->attributes_string() << std::endl;
    }
	return 0;
}
