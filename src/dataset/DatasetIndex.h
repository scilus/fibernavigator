#ifndef DATASETINDEX_H_
#define DATASETINDEX_H_

#include <algorithm>

class DatasetIndex2
{
public:
    DatasetIndex2() : m_index( BAD_INDEX ) {}
    DatasetIndex2( unsigned int index ) : m_index( index ) {}
    DatasetIndex2( const DatasetIndex2 &other ) : m_index( other.m_index ) {}

    // Functions
    bool isOk() const { return BAD_INDEX != m_index; }

    // Operators
    DatasetIndex2 &operator=( const DatasetIndex2 &other )
    {
        DatasetIndex2(other).swap(*this);
        return *this;
    }

    bool operator== ( const DatasetIndex2 &other ) const { return other.m_index == m_index; }
    bool operator!= ( const DatasetIndex2 &other ) const { return !(other == *this); }
    bool operator<  ( const DatasetIndex2 &other ) const { return m_index < other.m_index; }
    bool operator>  ( const DatasetIndex2 &other ) const { return other < *this; }
    bool operator<= ( const DatasetIndex2 &other ) const { return !(other < *this); }
    bool operator>= ( const DatasetIndex2 &other ) const { return !(*this < other); }
    
    // prefix ++
    DatasetIndex2 &operator++() { ++m_index; return *this; }
    // postfix ++
    DatasetIndex2 operator++(int) 
    {
        DatasetIndex2 result(*this);    // make a copy for result
        ++(*this);                      // Now use the prefix version to do the work
        return result;                  // return the copy (the old) value.
    }

    operator int()          const { return m_index; }
    operator long()         const { return m_index; }
    operator unsigned int() const { return m_index; }

private:
    void swap( DatasetIndex2 &other )
    {
        std::swap( m_index, other.m_index );
    }

private:
    static const unsigned int BAD_INDEX = 0;
    unsigned int m_index;
};

#endif //DATASETINDEX_H_
