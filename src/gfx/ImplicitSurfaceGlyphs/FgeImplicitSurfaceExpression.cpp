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
#include <string>
#include <vector>
#include <stack>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

/**
 * Structure to represent a token read in from the input stream.  The
 * tokenizer class returns these.
 */
struct token
{
    enum type
    {
        end_of_input,
        number, identifier,
        left_parenthesis, right_parenthesis,
        comma,
        add, subtract,
        multiply, divide,
        power
    };
    type token_type;
    string text;
    int position;
};

/**
 * Given a string as input, divide it into a list of tokens.
 *
 * @param input  the string to be tokenize
 * @param tokens  the stream of tokens created from the input
 */
static void tokenize(
    string const &input,
    vector< token > &tokens )
{
    tokens.clear();
    int state = 0;
    int start = 0;
    token new_token;
    for ( unsigned int position = 0; position <= input.size(); ++position )
    {
        int character = position < input.size() ? input[ position ] : -1;
        switch ( state )
        {
            case 0:
                start = position;
                if ( character >= '0' && character <= '9' )
                    state = 1;
                else if ( character == '.' )
                    state = 2;
                else if ( (character >= 'A' && character <= 'Z') ||
                          (character >= 'a' && character <= 'z') ||
                          character == '_' )
                    state = 7;
                else if ( character == '(' )
                {
                    new_token.token_type = token::left_parenthesis;
                    new_token.text.assign( 1, character );
                    new_token.position = start;
                    tokens.push_back( new_token );
                }
                else if ( character == ')' )
                {
                    new_token.token_type = token::right_parenthesis;
                    new_token.text.assign( 1, character );
                    new_token.position = start;
                    tokens.push_back( new_token );
                }
                else if ( character == ',' )
                {
                    new_token.token_type = token::comma;
                    new_token.text.assign( 1, character );
                    new_token.position = start;
                    tokens.push_back( new_token );
                }
                else if ( character == '+' )
                {
                    new_token.token_type = token::add;
                    new_token.text.assign( 1, character );
                    new_token.position = start;
                    tokens.push_back( new_token );
                }
                else if ( character == '-' )
                {
                    new_token.token_type = token::subtract;
                    new_token.text.assign( 1, character );
                    new_token.position = start;
                    tokens.push_back( new_token );
                }
                else if ( character == '*' )
                {
                    new_token.token_type = token::multiply;
                    new_token.text.assign( 1, character );
                    new_token.position = start;
                    tokens.push_back( new_token );
                }
                else if ( character == '/' )
                {
                    new_token.token_type = token::divide;
                    new_token.text.assign( 1, character );
                    new_token.position = start;
                    tokens.push_back( new_token );
                }
                else if ( character == '^' )
                {
                    new_token.token_type = token::power;
                    new_token.text.assign( 1, character );
                    new_token.position = start;
                    tokens.push_back( new_token );
                }
                else if ( character != ' ' && character != '\t' &&
                          character != '\r' && character != '\n' &&
                          character != -1 )
                    throw string( "Unexpected character" );
                break;
            case 1:
                if ( character == '.' )
                    state = 3;
                else if ( character == 'E' || character == 'e' )
                    state = 4;
                else if ( character < '0' || character > '9' )
                {
                    --position;
                    new_token.token_type = token::number;
                    new_token.text.assign( input.data() + start, position - start + 1 );
                    new_token.position = start;
                    tokens.push_back( new_token );
                    state = 0;
                }
                break;
            case 2:
                if ( character >= '0' && character <= '9' )
                    state = 3;
                else
                    throw string( "Invalid number" );
                break;
            case 3:
                if ( character == 'E' || character == 'e' )
                    state = 4;
                else if ( character < '0' || character > '9' )
                {
                    --position;
                    new_token.token_type = token::number;
                    new_token.text.assign( input.data() + start, position - start + 1 );
                    new_token.position = start;
                    tokens.push_back( new_token );
                    state = 0;
                }
                break;
            case 4:
                if ( character == '+' || character == '-' )
                    state = 5;
                else if ( character >= '0' && character <= '9' )
                    state = 6;
                else
                    throw string( "Invalid number" );
                break;
            case 5:
                if ( character >= '0' && character <= '9' )
                    state = 6;
                else
                    throw string( "Invalid number" );
                break;
            case 6:
                if ( character < '0' || character > '9' )
                {
                    --position;
                    new_token.token_type = token::number;
                    new_token.text.assign( input.data() + start, position - start + 1 );
                    new_token.position = start;
                    tokens.push_back( new_token );
                    state = 0;
                }
                break;
            case 7:
                if ( ( character < '0' || character > '9' ) &&
                     ( character < 'A' || character > 'Z' ) &&
                     ( character < 'a' || character > 'z' ) &&
                     character != '_' )
                {
                    --position;
                    new_token.token_type = token::identifier;
                    new_token.text.assign( input.data() + start, position - start + 1 );
                    new_token.position = start;
                    tokens.push_back( new_token );
                    state = 0;
                }
                break;
        }
    }
    new_token.token_type = token::end_of_input;
    new_token.text.clear();
    tokens.push_back( new_token );
}

// Forward declaration to make mutual recursion work
static string postfix_generate_parse_level_1(
	  string type_prefix,
    vector< token > const tokens,
    int &position );

/**
 * Parse expressions involving parenthetic expressions, functions, numbers
 * or other identifiers for variables.
 *
 * @param tokens  the list of tokens to be parsed
 * @param position  current position in the token list (will be updated)
 * @return  a string with the digested version of this subexpression
 */
static string postfix_generate_parse_level_5(
	string type_prefix,
	vector< token > const tokens,
    int &position )
{
    if ( tokens[ position ].token_type == token::left_parenthesis )
    {
        string result = postfix_generate_parse_level_1(type_prefix, tokens, ++position );
        if ( tokens[ position++ ].token_type != token::right_parenthesis )
            throw string( "Expected a closing parenthesis" );
        return result;
    }
    else if ( tokens[ position ].token_type == token::number )
        return tokens[ position++ ].text;
    else if ( tokens[ position ].token_type == token::identifier )
    {
        if ( tokens[ position ].text == "cos" ||
             tokens[ position ].text == "sin" ||
             tokens[ position ].text == "abs" )
        {
            string result = type_prefix + tokens[ position++ ].text + "( ";
            if ( tokens[ position++ ].token_type != token::left_parenthesis )
                throw string( "Expected an opening parenthesis" );
            result += postfix_generate_parse_level_1(type_prefix, tokens, position ) + " )";
            if ( tokens[ position++ ].token_type != token::right_parenthesis )
                throw string( "Expected a closing parenthesis" );
            return result;
        }
		else if (tokens[ position ].text == "tan" || tokens[ position ].text == "exp")
		{
            string result = type_prefix + tokens[ position++ ].text + "(inf, ";
            if ( tokens[ position++ ].token_type != token::left_parenthesis )
                throw string( "Expected an opening parenthesis" );
            result += postfix_generate_parse_level_1(type_prefix, tokens, position ) + " )";
            if ( tokens[ position++ ].token_type != token::right_parenthesis )
                throw string( "Expected a closing parenthesis" );
            return result;
		}
		else if (tokens[ position ].text == "sqrt" ||
					       tokens[position].text == "log" ||
								 tokens[ position ].text == "ln" ||
			                                               tokens[ position ].text == "acos")
		{
            string result = type_prefix + tokens[ position++ ].text + "(nan, inf, ";
            if ( tokens[ position++ ].token_type != token::left_parenthesis )
                throw string( "Expected an opening parenthesis" );
            result += postfix_generate_parse_level_1(type_prefix, tokens, position ) + " )";
            if ( tokens[ position++ ].token_type != token::right_parenthesis )
                throw string( "Expected a closing parenthesis" );
            return result;
		}
        else if ( tokens[ position ].text == "min" ||
                  tokens[ position ].text == "max" )
        {
            string result = type_prefix + tokens[ position++ ].text + "( ";
            if ( tokens[ position++ ].token_type != token::left_parenthesis )
                throw string( "Expected an opening parenthesis" );
            result += postfix_generate_parse_level_1(type_prefix, tokens, position ) + ", ";
            if ( tokens[ position++ ].token_type != token::comma )
                throw string( "Expected a comma" );
            result += postfix_generate_parse_level_1(type_prefix, tokens, position ) + " )";
            if ( tokens[ position++ ].token_type != token::right_parenthesis )
                throw string( "Expected a closing parenthesis" );
            return result;
        }
        else
            return tokens[ position++ ].text;
    }
    throw string( "Expected a number, identifier, function or parenthetic expression" );
}

/**
 * Parse expressions involving negation.
 *
 * @param tokens  the list of tokens to be parsed
 * @param position  current position in the token list (will be updated)
 * @return  a string with the digested version of this subexpression
 */
static string postfix_generate_parse_level_4(
	string type_prefix,
	vector< token > const tokens,
  int &position )
{
    if ( tokens[ position ].token_type == token::add )
    {
        ++position;
        return postfix_generate_parse_level_4(type_prefix, tokens, position );
    }
    else if ( tokens[ position ].token_type == token::subtract )
    {
        ++position;
        return type_prefix + "neg( " + postfix_generate_parse_level_4(type_prefix, tokens, position ) + " )";
    }
    return postfix_generate_parse_level_5(type_prefix, tokens, position );
}

/**
 * Parse expressions involving raising numbers to a power.
 *
 * @param tokens  the list of tokens to be parsed
 * @param position  current position in the token list (will be updated)
 * @return  a string with the digested version of this subexpression
 */
static string postfix_generate_parse_level_3(
	string type_prefix,
	vector< token > const tokens,
  int &position )
{
    string result = postfix_generate_parse_level_4(type_prefix, tokens, position );
    if ( tokens[ position ].token_type == token::power )
    {
        ++position;
				string to_power = postfix_generate_parse_level_3(type_prefix, tokens, position );
				float fnum = strtod(to_power.c_str(), 0);
				int inum = (int)fnum;
				if (inum >= 0 && fnum - (float)inum == 0.f)	//if integer
				{
					if (inum > 0)
						result = type_prefix + "pow" + to_power + "(" + result + ")";
					else
					{
						stringstream ss;
						ss << -inum;
						result = type_prefix + "powneg" + ss.str() + "(" + result + ")";
					}
				}
				else
					result = type_prefix + "pow(nan, inf, " + result + ", " + to_power + " )";
    }
    return result;
}

/**
 * Parse expressions involving multiplication or division.
 *
 * @param tokens  the list of tokens to be parsed
 * @param position  current position in the token list (will be updated)
 * @return  a string with the digested version of this subexpression
 */
static string postfix_generate_parse_level_2(
	string type_prefix,
  vector< token > const tokens,
  int &position )
{
    string result = postfix_generate_parse_level_3(type_prefix, tokens, position );
    for ( ; ; )
    {
        if ( tokens[ position ].token_type == token::multiply )
        {
            ++position;
            result = type_prefix + "mul( " + result + ", " + postfix_generate_parse_level_3(type_prefix, tokens, position ) + " )";
        }
        else if ( tokens[ position ].token_type == token::divide )
        {
            ++position;
            result = type_prefix + "div(inf, " + result + ", " + postfix_generate_parse_level_3(type_prefix, tokens, position ) + " )";
        }
        else
            break;
    }
    return result;
}

/**
 * Parse expressions involving addition or subtraction -- the lowest level
 * of precedence.
 *
 * @param tokens  the list of tokens to be parsed
 * @param position  current position in the token list (will be updated)
 * @return  a string with the digested version of this subexpression
 */
static string postfix_generate_parse_level_1(
	string type_prefix,
	vector< token > const tokens,
  int &position )
{
    string result = postfix_generate_parse_level_2(type_prefix, tokens, position );
    for ( ; ; )
    {
        if ( tokens[ position ].token_type == token::add )
        {
            ++position;
            result = type_prefix + "add( " + result + ", " + postfix_generate_parse_level_2(type_prefix, tokens, position ) + " )";
        }
        else if ( tokens[ position ].token_type == token::subtract )
        {
            ++position;
            result = type_prefix + "sub( " + result + ", " + postfix_generate_parse_level_2(type_prefix, tokens, position ) + " )";
        }
        else
            break;
    }
    return result;
}

/**
 * This is the main parser function that puts all of the above in a nice,
 * neat little package.  It takes a string with the desired expression,
 * tokenizes it, then parses it and returns a new string with the built up
 * CG code for it.
 *
 * @param input  a string with expression to parse
 * @return  a new string with the equivalent CG expression
 */
string postfix_generate_expression(
	string type_prefix,
  string const &input)
{
    vector< token > tokens;
    tokenize( input, tokens);
    int position = 0;
    return postfix_generate_parse_level_1(type_prefix, tokens, position);
}



/**********************************************************************************/

/* Same for evaluator expressions... */

static string evaluator_integer_pow_expression( string lhs, int inum )
{
	if (inum == 1)
	{
		return lhs;
	}
	else
	{
		int inumd2 = inum / 2;
		return "(" + evaluator_integer_pow_expression( lhs, inumd2 ) + "*" + evaluator_integer_pow_expression( lhs, inum - inumd2 ) + ")";
	}
}

// Forward declaration to make mutual recursion work
static string evaluator_parse_level_1(
    vector< token > const tokens,
    int &position );

static string evaluator_parse_level_5(
    vector< token > const tokens,
    int &position )
{
    if ( tokens[ position ].token_type == token::left_parenthesis )
    {
        string result = evaluator_parse_level_1( tokens, ++position );
        if ( tokens[ position++ ].token_type != token::right_parenthesis )
            throw string( "Expected a closing parenthesis" );
        return "(" + result + ")";
    }
    else if ( tokens[ position ].token_type == token::number )
        return tokens[ position++ ].text;
    else if ( tokens[ position ].token_type == token::identifier )
    {
        if ( tokens[ position ].text == "cos" ||
             tokens[ position ].text == "acos" ||
             tokens[ position ].text == "sin" ||
             tokens[ position ].text == "tan" ||
             tokens[ position ].text == "sqrt" ||
             tokens[ position ].text == "exp" ||
             tokens[ position ].text == "ln" ||
             tokens[ position ].text == "log" ||
             tokens[ position ].text == "abs" )
        {
            string result = tokens[ position++ ].text + "( ";
            if ( tokens[ position++ ].token_type != token::left_parenthesis )
                throw string( "Expected an opening parenthesis" );
            result += evaluator_parse_level_1( tokens, position ) + " )";
            if ( tokens[ position++ ].token_type != token::right_parenthesis )
                throw string( "Expected a closing parenthesis" );
            return result;
        }
        else if ( tokens[ position ].text == "min" ||
                  tokens[ position ].text == "max" )
        {
            string result = tokens[ position++ ].text + "( ";
            if ( tokens[ position++ ].token_type != token::left_parenthesis )
                throw string( "Expected an opening parenthesis" );
            result += evaluator_parse_level_1( tokens, position ) + ", ";
            if ( tokens[ position++ ].token_type != token::comma )
                throw string( "Expected a comma" );
            result += evaluator_parse_level_1( tokens, position ) + " )";
            if ( tokens[ position++ ].token_type != token::right_parenthesis )
                throw string( "Expected a closing parenthesis" );
            return result;
        }
        else
            return tokens[ position++ ].text;
    }
    throw string( "Expected a number, identifier, function or parenthetic expression" );
}



static string evaluator_parse_level_4(
    vector< token > const tokens,
    int &position )
{
    if ( tokens[ position ].token_type == token::add )
    {
        ++position;
        return evaluator_parse_level_4( tokens, position );
    }
    else if ( tokens[ position ].token_type == token::subtract )
    {
        ++position;
        return "-(" + evaluator_parse_level_4( tokens, position ) + ")";
    }
    return evaluator_parse_level_5( tokens, position );
}

static string evaluator_parse_level_3(
    vector< token > const tokens,
    int &position )
{
    string result = evaluator_parse_level_4( tokens, position );
    if ( tokens[ position ].token_type == token::power )
    {
        ++position;
				string to_power = evaluator_parse_level_3( tokens, position );
				float fnum = strtod(to_power.c_str(), 0);
				int inum = (int)fnum;
				if (inum >= 0 && fnum - (float)inum == 0.f)	//if integer
				{
					if (inum >= 0)
						result = "pow" + to_power + "(" + result + ")";
					else
					{
						stringstream ss;
						ss << -inum;
						result = "powneg" + ss.str() + "(" + result + ")";
					}
				}
				else
					result = "pow( " + result + ", " + to_power + " )";
    }
    return result;
}


static string evaluator_parse_level_2(
    vector< token > const tokens,
    int &position )
{
    string result = evaluator_parse_level_3( tokens, position );
    for ( ; ; )
    {
        if ( tokens[ position ].token_type == token::multiply )
        {
            ++position;
            result = result + "*" + evaluator_parse_level_3( tokens, position );
        }
        else if ( tokens[ position ].token_type == token::divide )
        {
            ++position;
            result = result + "/" + evaluator_parse_level_3( tokens, position );
        }
        else
            break;
    }
    return result;
}


static string evaluator_parse_level_1(
    vector< token > const tokens,
    int &position )
{
    string result = evaluator_parse_level_2( tokens, position );
    for ( ; ; )
    {
        if ( tokens[ position ].token_type == token::add )
        {
            ++position;
            result = result + " + " + evaluator_parse_level_2( tokens, position );
        }
        else if ( tokens[ position ].token_type == token::subtract )
        {
            ++position;
            result = result + " - " + evaluator_parse_level_2( tokens, position );
        }
        else
            break;
    }
    return result;
}

string evaluator_generate_expression(
    string const &input )
{
    vector< token > tokens;
    tokenize( input, tokens );
    int position = 0;
    return evaluator_parse_level_1( tokens, position );
}


/* for making int power expressions */
void make_pow_helper(
    int const power,
    ostringstream &ss )
{
    if ( power <= 1 )
        return;
    else if ( power % 2 == 0 )
    {
        make_pow_helper( power / 2, ss );
        ss << "    float x" << power << " = x" << ( power / 2 ) << " * x" << ( power / 2 ) << ";" << endl;
        return;
    }
    else if ( power % 2 == 1 )
    {
        make_pow_helper( power - 1, ss );
        ss << "    float x" << power << " = x1 * x" << ( power - 1 ) << ";" << endl;
    }
}

string make_pow(
    int const power )
{
		int abspower = power < 0 ? -power : power;
		ostringstream ss;
		if (power < 0)
		{
			ss << "float powneg" << abspower << "( float x1 )" << endl;
		}
		else
		{
	    ss << "float pow" << abspower << "( float x1 )" << endl;
		}
    ss << "{" << endl;
    make_pow_helper( abspower, ss );
		if (power < 0)
		{
			ss << "    return 1/x" << abspower << ";" << endl;
		}
		else
		{
	    ss << "    return x" << abspower << ";" << endl;
		}

    ss << "}" << endl;
    ss << endl;
    return ss.str();
}

/* for making autodiff power expressions */
void make_autodiff_pow_helper(
    int const power,
    ostringstream &ss )
{
    if ( power <= 1 )
        return;
    else if ( power % 2 == 0 )
    {
        make_autodiff_pow_helper( power / 2, ss );
        ss << "    autodiff x" << power << " = admul(x" << ( power / 2 ) << ", x" << ( power / 2 ) << ");" << endl;
        return;
    }
    else if ( power % 2 == 1 )
    {
        make_autodiff_pow_helper( power - 1, ss );
        ss << "    autodiff x" << power << " = admul(x1, x" << ( power - 1 ) << ");" << endl;
    }
}

string make_autodiff_pow(
    int const power )
{
		int abspower = power < 0 ? -power : power;

		ostringstream ss;
		if (power < 0)
		{
			ss << "autodiff adpowneg" << abspower << "( autodiff x1 )" << endl;
		}
		else
		{
	    ss << "autodiff adpow" << abspower << "( autodiff x1 )" << endl;
		}
    ss << "{" << endl;
    make_autodiff_pow_helper( abspower, ss );
		if (power < 0)
		{
			ss << "    return adrcp(x" << abspower << ");" << endl;
		}
		else
		{
	    ss << "    return x" << abspower << ";" << endl;
		}
    ss << "}" << endl;
    ss << endl;
    return ss.str();
}


string make_interval_pow(
    int const power )
{
		int abspower = power < 0 ? -power : power;
    ostringstream ss;

		if (power < 0)
		{
			ss << "float ipowneg" << abspower << "(float inf, float a ){ return 1/pow" << abspower << "(a); }" << endl << endl;
			ss << "interval ipowneg" << abspower << "(float inf, interval i )" << endl;
		}
		else
		{
	    ss << "float ipow" << abspower << "( float a ){ return pow" << abspower << "(a); }" << endl << endl;
	    ss << "interval ipow" << abspower << "( interval i )" << endl;
		}
    ss << "{" << endl;

		if (power == 2)
		{
			ss << "  return isqr(i);" << endl;
		}
		else
		{
			ss << "  float iplo = pow" << abspower << "(i.x);" << endl;
			ss << "  float iphi = pow" << abspower << "(i.y);" << endl;
			ss << "  interval ip = interval(min(iplo, iphi), max(iplo, iphi));" << endl;

			if (!(power & 1))
			{
				ss << "  ip.x = (i.x * i.y < 0) ? 0 : ip.x;" << endl;
			}

		if (power < 0)
		{
			ss << "    return ircp(inf, ip" << abspower << ");" << endl;
		}
		else
		{
	    ss << "    return ip;" << endl;
		}
		}
    ss << "}" << endl;
    ss << endl;
    return ss.str();
}

string make_adia_pow(
    int const power )
{
		int abspower = power < 0 ? -power : power;
    ostringstream ss;

	  ss << "adi adipow" << abspower << "( adi i )" << endl;
    ss << "{" << endl;

		ss << "  adi r;" << endl;
		ss << "  r.xy = ipow" << abspower << "(i.xy);\n";

		if (power > 2)
			ss << "  r.zw = imul(imul(" << power << ", ipow"<< power-1 << "(i.xy)), i.zw);\n";
		else if (power == 2)
			ss << "  r.zw = imul(2, imul(i.xy, i.zw));\n";

	  ss << "  return r;" << endl;

		ss << "}" << endl;
    ss << endl;
    return ss.str();
}

/* for making int power expressions */
void make_affine_pow_helper(
    int const power,
    ostringstream &ss )
{
    if ( power <= 1 )
        return;
    else if ( power % 2 == 0 )
    {
        make_affine_pow_helper( power / 2, ss );
        ss << "    raf x" << power << " = raf_mul(x" << ( power / 2 ) << ", x" << ( power / 2 ) << ");" << endl;
        return;
    }
    else if ( power % 2 == 1 )
    {
        make_affine_pow_helper( power - 1, ss );
        ss << "    raf x" << power << " = raf_mul(x1, x" << ( power - 1 ) << ");" << endl;
    }
}

string make_affine_pow(
    int const power )
{
		int abspower = power < 0 ? -power : power;
		ostringstream ss;
		if (power < 0)
		{
			ss << "float raf_powneg" << abspower << "(float inf, float a ){ return 1/pow" << abspower << "(a); }" << endl << endl;
			ss << "raf raf_powneg" << abspower << "( raf x1 )" << endl;
		}
		else
		{
	    ss << "float raf_pow" << abspower << "( float a ){ return pow" << abspower << "(a); }" << endl << endl;
	    ss << "raf raf_pow" << abspower << "( raf x1 )" << endl;
		}
    ss << "{" << endl;
    make_affine_pow_helper( abspower, ss );
		if (power < 0)
		{
			ss << "    return raf_rcp(x)" << abspower << ";" << endl;
		}
		else
		{
	    ss << "    return x" << abspower << ";" << endl;
		}

    ss << "}" << endl;
    ss << endl;
    return ss.str();
}
