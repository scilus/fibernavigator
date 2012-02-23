/*
 * DatasetHelper.h
 *
 *  Created on: 27.07.2008
 *      Author: ralph
 */
#ifndef DATASETHELPER_H_
#define DATASETHELPER_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/textfile.h"
#include "wx/file.h"
#include "wx/image.h"
#include "wx/wfstream.h"
#include "wx/datstrm.h"
#include "wx/txtstrm.h"
#include "wx/xml/xml.h"

#include <vector>

#include "DatasetInfo.h"

#include "../gui/SelectionObject.h"

#include "SplinePoint.h"

#include "AnatomyHelper.h"
#include "../gfx/ShaderHelper.h"

#include "../misc/lic/TensorField.h"
#include "../misc/Fantom/FMatrix.h"

class DatasetInfo;
class AnatomyHelper;
class ShaderHelper;
class SplinePoint;
class SelectionObject;
class Fibers;
class FibersGroup;
class TensorField;
class Surface;

class DatasetHelper 
{
public:
    // Constructor/destructor
    DatasetHelper();
    virtual ~DatasetHelper();

    // Functions
    void   deleteAllPoints();
    Vector mapMouse2World( const int i_x, const int i_y,GLdouble i_projection[16], GLint i_viewport[4], GLdouble i_modelview[16]);
    Vector mapMouse2WorldBack( const int i_x, const int i_y,GLdouble i_projection[16], GLint i_viewport[4], GLdouble i_modelview[16]);   

    /*
     * Called from MainFrame when a kdTree thread signals it's finished
     */
    void treeFinished();

    std::vector< float >* getVectorDataset();
    TensorField* getTensorField();

    void doLicMovie       ( int i_mode );
    void createLicSliceSag( int i_slize );
    void createLicSliceCor( int i_slize );
    void createLicSliceAxi( int i_slize );
    void licMovieHelper();

    /////////////////////////////////////////////////////////////////////////////////
    // general info about the datasets
    /////////////////////////////////////////////////////////////////////////////////
    std::vector<Vector>   m_rulerPts;
    bool                  m_isRulerToolActive;
    double                m_rulerFullLength;
    double                m_rulerPartialLength;

    unsigned int m_countFibers;

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
