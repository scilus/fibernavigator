#ifndef DATASETHELPER_H_
#define DATASETHELPER_H_

#include <vector>

#include "../gui/SelectionObject.h"

#include "SplinePoint.h"

class SplinePoint;
class SelectionObject;

class DatasetHelper 
{
public:
    // Constructor/destructor
    DatasetHelper();
    virtual ~DatasetHelper();

    // Functions
    void   deleteAllPoints();

    /////////////////////////////////////////////////////////////////////////////////
    // state variables for menu entries
    /////////////////////////////////////////////////////////////////////////////////
    bool  m_selBoxChanged;

    /////////////////////////////////////////////////////////////////////////////////
    // pointers to often used objects
    /////////////////////////////////////////////////////////////////////////////////
    SplinePoint*     m_lastSelectedPoint;
    SelectionObject* m_lastSelectedObject;
};

#endif /* DATASETHELPER_H_ */
