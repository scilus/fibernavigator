/////////////////////////////////////////////////////////////////////////////
// Name:            MenuBar.h
// Author:          GGirard
// Creation Date:   03/12/2010
//
// Description: MainFrame MenuBar
//
// Last modifications:
//      
/////////////////////////////////////////////////////////////////////////////

#ifndef MENUBAR_H_
#define MENUBAR_H_

#include <wx/wx.h>
#include <wx/menu.h>

class MainFrame;
class MenuBar : public wxMenuBar
{
public:
    MenuBar();
    void initMenuBar(MainFrame *mf);
    void updateMenuBar(MainFrame *mf);    

public:
    wxMenu      *m_menuFile;
        wxMenuItem  *m_itemLoad;
        wxMenuItem  *m_itemLoadAsPeaks;
        wxMenu      *m_menuNewAnatomy;
            wxMenuItem  *m_itemNewAnatomyByte;
            wxMenuItem  *m_itemNewAnatomyRGB;
        wxMenu      *m_menuScreenshot;
            wxMenuItem  *m_itemScreenshot;
            wxMenu      *m_menuScreenshotResolution;
                wxMenuItem  *m_itemScreenshotResolution2048;
                wxMenuItem  *m_itemScreenshotResolution4096;
                wxMenuItem  *m_itemScreenshotResolution8192; 
                wxMenuItem  *m_itemScreenshotResolution16384;            
            wxMenu      *m_menuScreenshotLineWidth;
                wxMenuItem  *m_itemScreenshotLineWidth1;
                wxMenuItem  *m_itemScreenshotLineWidth2;
                wxMenuItem  *m_itemScreenshotLineWidth4;
                wxMenuItem  *m_itemScreenshotLineWidth8;
            wxMenuItem  *m_itemScreenshotTransparencySaved;
            wxMenuItem  *m_itemScreenshotTransparencyInverted;
        wxMenuItem  *m_itemSaveSCN;
        wxMenuItem  *m_itemSaveSelectedFibers;
        wxMenuItem  *m_itemSaveSelectedSurface;
        wxMenuItem  *m_itemSaveSelectedDataset;
        wxMenuItem  *m_itemQuit; 

    wxMenu      *m_menuView;
        wxMenu      *m_menuShowSlices;
            wxMenuItem  *m_itemToggleShowAxial;
            wxMenuItem  *m_itemToggleShowCoronal;
            wxMenuItem  *m_itemToggleShowSagittal;
        wxMenu      *m_menuNavigate;
            wxMenuItem  *m_itemNavigateSlizeX;
            wxMenuItem  *m_itemNavigateSlizeY;
            wxMenuItem  *m_itemNavigateSlizeZ;
        wxMenu      *m_menuRotate;
            wxMenuItem  *m_itemRotateZ;
            wxMenuItem  *m_itemRotateY;
            wxMenuItem  *m_itemRotateX;
        wxMenuItem  *m_itemReset;
        wxMenuItem  *m_itemLeft;
        wxMenuItem  *m_itemRight;
        wxMenuItem  *m_itemSuperior;
        wxMenuItem  *m_itemInferior;
        wxMenuItem  *m_itemAnterior;
        wxMenuItem  *m_itemPosterior;
        wxMenuItem  *m_itemToggleShowCrosshair;
        wxMenuItem  *m_itemToggleShowAxes;
        wxMenuItem  *m_itemLockSlizes;
        wxMenuItem  *m_itemLockScene;

    wxMenu      *m_menuVoi;
        wxMenuItem  *m_itemNewSelectionBox;
        wxMenuItem  *m_itemNewSelectionEllipsoid;

    wxMenu      *m_menuFibers;
        wxMenuItem  *m_itemResetFibersColors;
        wxMenuItem  *m_itemToggleUseTransparency;
        wxMenuItem  *m_itemToggleInvertFibersSelection;
        wxMenuItem  *m_itemToggleUseFakeTubes;
        wxMenuItem  *m_itemToggleUseGeometryShader;

    wxMenu      *m_menuOptions; 
        wxMenu      *m_menuRuler;
            wxMenuItem  *m_itemToggleRuler;
            wxMenuItem  *m_itemRulerClear;
            wxMenuItem  *m_itemRulerAdd;
            wxMenuItem  *m_itemRulerRemove;
        wxMenu      *m_menuDrawer;
            wxMenuItem  *m_itemToggleDrawer;
            wxMenuItem  *m_itemToggleDrawRound;
            wxMenuItem  *m_itemToggleDraw3d;
            wxMenuItem  *m_itemDrawColorPicker;
            wxMenu  *m_menuStrokeSize;
                wxMenuItem  *m_itemDrawS1;
                wxMenuItem  *m_itemDrawS2;
                wxMenuItem  *m_itemDrawS3;
                wxMenuItem  *m_itemDrawS4;
                wxMenuItem  *m_itemDrawS5;
                wxMenuItem  *m_itemDrawS7;
                wxMenuItem  *m_itemDrawS10;
            wxMenuItem  *m_itemDrawPen;
            wxMenuItem  *m_itemDrawEraser;
        wxMenu      *m_menuColorMaps;
            wxMenuItem  *m_itemGray;
            wxMenuItem  *m_itemBlueGreenPurple;
            wxMenuItem  *m_itemRainbow; 
            wxMenuItem  *m_itemHotIron;
            wxMenuItem  *m_itemRedYellow;
            wxMenuItem  *m_itemBlueLightblue;

        wxMenuItem  *m_itemToggleDrawPoints;
        wxMenuItem  *m_itemToggleDrawVectors;
        wxMenuItem  *m_itemToggleLighting;
        wxMenuItem  *m_itemToggleClearToBlack;
        wxMenuItem  *m_itemToggleBlendTextureOnMesh;
        wxMenuItem  *m_itemToggleFilterISO;
        wxMenuItem  *m_itemToggleNormal;

     wxMenu         *m_menuHelp;
        wxMenuItem  *m_itemAbout;
        wxMenuItem  *m_itemKeyboardShortcuts;
        wxMenuItem  *m_itemWarningsInfo;
};

#endif