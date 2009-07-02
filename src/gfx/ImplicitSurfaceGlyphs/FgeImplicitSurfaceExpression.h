//Copyright (c) 2008 Aaron Knoll, Younis Hijazi, Andrew Kensler, Mathias Schott, Charles Hansen and Hans Hagen
//
// edited 2009 by Mario Hlawitschka and Alexander Wiebel
//
//Permission is hereby granted, free of charge, to any person
//obtaining a copy of this software and associated documentation
//files (the "Software"), to deal in the Software without
//restriction, including without limitation the rights to use,
//copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the
//Software is furnished to do so, subject to the following
//conditions:
//
//The above copyright notice and this permission notice shall be
//included in all copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
//OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
//HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
//WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
//OTHER DEALINGS IN THE SOFTWARE.
#ifndef EXPRESSION_HXX
#define EXPRESSION_HXX

#include <string>

std::string postfix_generate_expression(std::string type_prefix, std::string const &input);

std::string evaluator_generate_expression(std::string const &input);

std::string make_pow(int const power);

std::string make_interval_pow(int const power);

std::string make_autodiff_pow(int const power);

std::string make_adia_pow(int const power);

std::string make_affine_pow(int const power);


#endif
