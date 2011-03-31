#ifndef __binio_hh
#define __binio_hh

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

// ---------------------------------------------------------------

template<typename T> inline std::ostream&
binwrite_raw( std::ostream& out, const T* t, size_t cnt = 1 )
{
    out.write( (char*)t, sizeof(T)*cnt );
    return out;
}

template<typename T> inline std::istream&
binread_raw( std::istream& in, T* t, size_t cnt = 1 )
{
    unsigned int s = sizeof(T)*cnt;

    in.read( (char*)t, s );
    return in;
}

// ---------------------------------------------------------------

inline std::ostream& binwrite( std::ostream& out, unsigned int n )
{
    return binwrite_raw( out, &n );
}

inline std::ostream& binwrite( std::ostream& out, unsigned char n )
{
    return binwrite_raw( out, &n );
}

inline std::ostream& binwrite( std::ostream& out, int n )
{
    return binwrite_raw( out, &n );
}

inline std::ostream& binwrite( std::ostream& out, double n )
{
    return binwrite_raw( out, &n );
}

// inline std::ostream& binwrite( std::ostream& out, bool b )
// {
//   return binwrite_raw( out, &b );
// }


inline std::ostream& binwrite( std::ostream& out, const std::string& s )
{
    unsigned int len = s.length();

    binwrite_raw( out, &len );

    std::string::const_iterator i = s.begin();

    while( len-- )
	out.put( *(i++) );

    return out;
}



// ---------------------------------------------------------------

inline std::istream& binread( std::istream& in, unsigned int &n )
{
    return binread_raw( in, &n );
}

inline std::istream& binread( std::istream& in, int &n )
{
    return binread_raw( in, &n );
}

inline std::istream& binread( std::istream& in, double &n )
{
    return binread_raw( in, &n );
}

// inline std::istream& binread( std::istream& in, bool &b )
// {
//     return binread_raw( in, &b );
// }

inline std::istream& binread( std::istream& in, std::string& s )
{
    unsigned int len;

    binread_raw( in, &len );

    s.resize( len );
    std::string::iterator i = s.begin();

    while( len-- )
	in.get( *(i++) );

    return in;
}


// ---------------------------------------------------------------

template<typename T1,typename T2> inline std::ostream&
binwrite( std::ostream& out, const std::pair<T1,T2>& v )
{
 	binwrite( out, v.first );
 	binwrite( out, v.second );

    return out;
}
template<typename T1,typename T2> inline std::istream&
binread( std::istream& in, std::pair<T1,T2>& v )
{
    binread(in,v.first);
    binread(in,v.second);
    return in;
}

// ---------------------------------------------------------------

template<typename T> inline std::ostream&
binwrite( std::ostream& out, const std::vector<T>& v )
{
    unsigned int size = v.size();
    binwrite_raw( out, &size );

    for( typename std::vector<T>::const_iterator i=v.begin(); i!=v.end(); ++i )
 	binwrite( out, *i );

    return out;
}

template<typename T> inline std::istream&
binread( std::istream& in, std::vector<T>& v )
{
    unsigned int size;
    binread_raw( in, &size );

    v.resize( size );

    for( typename std::vector<T>::iterator i=v.begin(); i!=v.end(); ++i )
	binread( in, *i );

    return in;
}

// ---------------------------------------------------------------

#endif //__binio_hh
