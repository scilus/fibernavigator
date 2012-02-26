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
    // general info about the datasets
    /////////////////////////////////////////////////////////////////////////////////
    std::vector<Vector>   m_rulerPts;
    bool                  m_isRulerToolActive;
    double                m_rulerFullLength;
    double                m_rulerPartialLength;

    unsigned int m_countFibers;  // TODO: Move to DatasetManager and remove me once selection is fixed

    bool m_scnFileLoaded;
    bool m_surfaceIsDirty;

    /////////////////////////////////////////////////////////////////////////////////
    // state variables for rendering
    /////////////////////////////////////////////////////////////////////////////////
    //! if set the shaders will be reloaded during next render() call
    bool      m_scheduledReloadShaders;
    // the screenshot button has been pressed, next render pass it will be executed
    bool      m_scheduledScreenshot;

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

    wxString m_scenePath;
    wxString m_scnFileName;
    wxString m_screenshotPath;
    wxString m_screenshotName;

    /////////////////////////////////////////////////////////////////////////////////
    // pointers to often used objects
    /////////////////////////////////////////////////////////////////////////////////
    SelectionObject* m_boxAtCrosshair;
    SplinePoint*     m_lastSelectedPoint;
    SelectionObject* m_lastSelectedObject;
};

#define ID_KDTREE_FINISHED    50

#endif /* DATASETHELPER_H_ */
