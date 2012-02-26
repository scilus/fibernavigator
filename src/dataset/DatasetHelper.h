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

    /*
     * Called from MainFrame when a kdTree thread signals it's finished
     */
    void treeFinished();

    /////////////////////////////////////////////////////////////////////////////////
    // state variables for rendering
    /////////////////////////////////////////////////////////////////////////////////

    float     m_frustum[6][4]; // Contains the information of the planes forming the frustum.
    /////////////////////////////////////////////////////////////////////////////////
    // state variables for menu entries
    /////////////////////////////////////////////////////////////////////////////////
    bool  m_clearToBlack;
    bool  m_filterIsoSurf;
    int   m_colorMap;
    bool  m_showColorMapLegend;

    bool  m_boxLockIsOn;
    int   m_threadsActive;

    bool  m_texAssigned;
    bool  m_selBoxChanged;

    int   m_geforceLevel;

    /////////////////////////////////////////////////////////////////////////////////
    // pointers to often used objects
    /////////////////////////////////////////////////////////////////////////////////
    SelectionObject* m_boxAtCrosshair;
    SplinePoint*     m_lastSelectedPoint;
    SelectionObject* m_lastSelectedObject;
};

#define ID_KDTREE_FINISHED    50

#endif /* DATASETHELPER_H_ */
