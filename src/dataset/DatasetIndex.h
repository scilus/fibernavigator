#ifndef DATASETINDEX_H_
#define DATASETINDEX_H_

#include <algorithm>

class DatasetIndex
{
public:
    DatasetIndex() : m_index( BAD_INDEX ) {}
    DatasetIndex( unsigned int index ) : m_index( index ) {}
    DatasetIndex( const DatasetIndex &other ) : m_index( other.m_index ) {}

    // Functions
    bool isOk() const { return BAD_INDEX != m_index; }

    // Operators
    DatasetIndex &operator=( const DatasetIndex &other )
    {
        DatasetIndex(other).swap(*this);
        return *this;
    }

    bool operator== ( const DatasetIndex &other ) const { return other.m_index == m_index; }
    bool operator!= ( const DatasetIndex &other ) const { return !(other == *this); }
    bool operator<  ( const DatasetIndex &other ) const { return m_index < other.m_index; }
    bool operator>  ( const DatasetIndex &other ) const { return other < *this; }
    bool operator<= ( const DatasetIndex &other ) const { return !(other < *this); }
    bool operator>= ( const DatasetIndex &other ) const { return !(*this < other); }
    
    // prefix ++
    DatasetIndex &operator++() { ++m_index; return *this; }
    // postfix ++
    DatasetIndex operator++(int) 
    {
        DatasetIndex result(*this);    // make a copy for result
        ++(*this);                      // Now use the prefix version to do the work
        return result;                  // return the copy (the old) value.
    }

    operator int()          const { return m_index; }
    operator long()         const { return m_index; }
    operator unsigned int() const { return m_index; }

private:
    void swap( DatasetIndex &other )
    {
        std::swap( m_index, other.m_index );
    }

private:
    static const unsigned int BAD_INDEX = 0;
    unsigned int m_index;
};

#endif //DATASETINDEX_H_
