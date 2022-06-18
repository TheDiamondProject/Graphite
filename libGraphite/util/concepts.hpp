// Copyright (c) 2022 Tom Hancocks
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

#pragma once

#include <concepts>
#include <string>

#include "libGraphite/rsrc/resource.hpp"
#include "libGraphite/data/data.hpp"

namespace graphite
{
    template<class From, class To>
    concept convertible_to = std::is_convertible_v<From, To> && requires {
        static_cast<To>(std::declval<From>());
    };

    template<class T, class... Args>
    concept constructible_from = std::destructible<T> && std::is_constructible_v<T, Args...>;

    template<class T>
    concept move_constructible = constructible_from<T, T> && convertible_to<T, T>;

    template<class T>
    concept resource_type = requires(const T& resource) {
        requires constructible_from<T, const data::block&, rsrc::resource::identifier, const std::string&>;
        requires move_constructible<T>;
        { T::type_code() } -> std::same_as<std::string>;
    };
}