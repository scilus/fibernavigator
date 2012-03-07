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
    //RTT vars
    /////////////////////////////////////////////////////////////////////////////////
    bool m_isRTTReady;
    bool m_isRTTDirty;
    
	bool m_isRTTActive;
    bool m_isRandomSeeds;
	bool m_interpolateTensors;
	bool m_isFileSelected;
};

#endif /* DATASETHELPER_H_ */
