#include "FMultiIndex.h"

std::ostream& operator<<(std::ostream& os, const FMultiIndex& mi)
{
  os << "[ ";
  for(unsigned int i=0; i< mi.order(); ++i)
  {
    os << mi[i] << " ";
  }
  os << "]";
  return os;
}

