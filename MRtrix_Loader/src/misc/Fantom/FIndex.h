//---------------------------------------------------------------------------
//
// Project:   FAnToM
// Module:    $RCSfile: FIndex.hh,v $
// Language:  C++
// Date:      $Date: 2004/07/20 07:27:53 $
// Author:    $Author: wiebel $
// Version:   $Revision: 1.15 $
//
//---------------------------------------------------------------------------

#ifndef __FIndex_hh
#define __FIndex_hh

#include "stdAliases.h"
#include "FString.h"
#include "FException.h"


//===========================================================================

/**
 * Undocumented.
 */
class FIndex
{
public:

  /// invalid index value
  static const positive invalid;

  /// return an invalid index
  FIndex();

  ///  Undocumented.
  FIndex(positive value);

  ///  Undocumented.
  FIndex(const FIndex& id);

  ///  Undocumented.
  ~FIndex();

  ///  Undocumented.
  //const FString& getClassName() const;

  /**
   * {\bf Description:}\\
   * Returns the positive value corresponding to the index.
   * \pre
   * Index must be valid.
   * \post
   * none
   * \exception
   * FException: invalid index.
   */
  positive getIndex() const;

  ///  Undocumented.
//  operator unsigned long( void ) const;

  ///  Undocumented.
  operator positive( void )const;

  ///  Undocumented.
  FIndex& operator=(const FIndex& id);
  ///  Undocumented.
  FIndex& operator=(positive value);

  ///  Undocumented.
  bool operator==(const FIndex& id) const;
  ///  Undocumented.
  bool operator==(positive value) const;

  ///  Undocumented.
  bool operator!=(const FIndex& id) const;
  ///  Undocumented.
  bool operator!=(positive value) const;

  ///  Undocumented.
  bool operator<(const FIndex& id) const;
  ///  Undocumented.
  bool operator<(positive value) const;

  ///  Undocumented.
  bool operator<=(const FIndex& id) const;
  ///  Undocumented.
  bool operator<=(positive value) const;

  ///  Undocumented.
  bool operator>(const FIndex& id) const;
  ///  Undocumented.
  bool operator>(positive value) const;

  ///  Undocumented.
  bool operator>=(const FIndex& id) const;
  ///  Undocumented.
  bool operator>=(positive value) const;

  ///  Undocumented.
  FIndex operator+(const FIndex& id) const;
  ///  Undocumented.
  FIndex operator+(positive value) const;

  ///  Undocumented.
  FIndex & operator+=(const FIndex& id);
  ///  Undocumented.
  FIndex & operator+=(positive value);

  ///  Undocumented.
  FIndex operator-(const FIndex& id) const;
  ///  Undocumented.
  FIndex operator-(positive value) const;

  ///  Undocumented.
  FIndex & operator-=(const FIndex& id);
  ///  Undocumented.
  FIndex & operator-=(positive value);

  ///  Undocumented.
  FIndex& operator++();
  ///  Undocumented.
  FIndex& operator++(int);

  ///  Undocumented.
  FIndex& operator--();
  ///  Undocumented.
  FIndex& operator--(int);

  ///  Undocumented.
  FIndex operator*(positive value) const;

  ///  Undocumented.
  friend std::ostream&  operator<<(std::ostream& os, const FIndex& inIndex);
  ///  Undocumented.
  friend std::istream&  operator>>(std::istream& is, FIndex& inIndex);

  ///  Undocumented.
  bool isValid() const;

  ///  Undocumented.
  void setToInvalid();

private:

  lpositive index;
};

//===========================================================================
#ifndef OUTLINE
#include "FIndex.icc"
#endif
//===========================================================================

#endif // __FIndex_hh

