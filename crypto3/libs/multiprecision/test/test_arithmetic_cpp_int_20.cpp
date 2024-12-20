///////////////////////////////////////////////////////////////
//  Copyright 2012 John Maddock. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt

#include <nil/crypto3/multiprecision/boost_backends/cpp_int_modular.hpp>

#include "test_arithmetic.hpp"

template <unsigned MinBits, unsigned MaxBits, boost::multiprecision::cpp_integer_type SignType, class Allocator, boost::multiprecision::expression_template_option ExpressionTemplates>
struct is_twos_complement_integer<boost::multiprecision::number<boost::multiprecision::cpp_int_modular_backend<MinBits, MaxBits, SignType, boost::multiprecision::checked, Allocator>, ExpressionTemplates> > : public std::integral_constant<bool, false>
{};

template <>
struct related_type<boost::multiprecision::cpp_int>
{
   typedef boost::multiprecision::number<boost::multiprecision::cpp_int_modular_backend<4096> > type;
};

int main()
{
   test<boost::multiprecision::cpp_int>();
   return boost::report_errors();
}
