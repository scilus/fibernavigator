#ifndef FIBERSGROUP_H_
#define FIBERSGROUP_H_

#include "DatasetInfo.h"
#include "Fibers.h"

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <string>
#include <vector>

/**
 * This class represents a group container for sets of fibers.
 * It is useful to load several sets of fibers and can be used to
 * apply a characteristic to all set of fibers that are in the group.
 */
class FibersGroup : public DatasetInfo
{
public:
    FibersGroup();
    virtual ~FibersGroup();

    bool    isOneBtnToggled()
    {
        return m_isIntensityToggled && m_isOpacityToggled && m_isMinMaxLengthToggled 
            && m_isSubsamplingToggled && m_isColorModeToggled;
    }

    void    save( wxString filename );
    bool    save( wxXmlNode *pNode ) const;

    void    saveDMRI( wxString filename );

    void    updateGroupFilters();
    void    resetFibersColor();
    void    invertFibers();
    void    useFakeTubes();
    void    useTransparency();

    void    resetAllValues();

    void    fibersLocalColoring();
    void    fibersNormalColoring();

    void    generateGlobalFiberVolume(std::vector<Anatomy*> vAnatomies);

    void    OnToggleVisibleBtn();
    void    OnToggleIntensityBtn();
    void    OnToggleOpacityBtn();
    void    OnToggleMinMaxLengthBtn();
    void    OnToggleSubsamplingBtn();
    void    OnToggleCrossingFibersBtn();
    void    OnToggleColorModeBtn();
    void    OnToggleLocalColoring();
    void    OnToggleNormalColoring();
    void    OnClickGenerateFiberVolumeBtn();
    void    OnClickApplyBtn();
    void    OnClickCancelBtn();

    // Empty derived methods
    bool    load( wxString  filename ) { return false; };
    void    draw()                      {};
    void    smooth()                    {};
    void    flipAxis( AxisType i_axe )  {};
    void    drawVectors()               {};
    void    generateTexture()           {};
    void    generateGeometry()          {};
    void    initializeBuffer()          {};

    GLuint  getGLuint( ) { return 0; };

    virtual void createPropertiesSizer( PropertiesWindow *pParent );
    virtual void updatePropertiesSizer();

private:
    FibersGroup(const FibersGroup &);
    FibersGroup &operator=(const FibersGroup &);

private: 
    // Variables
    bool m_isIntensityToggled;
    bool m_isOpacityToggled;
    bool m_isMinMaxLengthToggled;
    bool m_isSubsamplingToggled;
    bool m_isColorModeToggled;
    bool m_isCrossingFibersToggled;

    bool m_isNormalColoringStateChanged;
    bool m_isLocalColoringStateChanged;

    std::string intToString( const int number );

    // GUI members
    wxButton *m_pBtnIntensity;
    wxButton *m_pBtnOpacity;
    wxButton *m_pBtnMinMaxLength;
    wxButton *m_pBtnSubsampling;
    wxButton *m_pBtnColorMode;
    wxToggleButton *m_pToggleLocalColoring;
    wxToggleButton *m_pToggleNormalColoring;
    wxToggleButton *m_pToggleInterFibers;

    wxButton       *m_pApplyBtn;
    wxButton       *m_pCancelBtn;

    wxButton       *m_pBtnGeneratesDensityVolume;
    wxSlider       *m_pSliderFibersFilterMin;
    wxSlider       *m_pSliderFibersFilterMax;
    wxSlider       *m_pSliderFibersSampling;
    wxSlider       *m_pSliderInterFibersThickness;
    wxRadioButton  *m_pRadNormalColoring;
    wxRadioButton  *m_pRadDistanceAnchoring;
    wxRadioButton  *m_pRadMinDistanceAnchoring;
    wxRadioButton  *m_pRadCurvature;
    wxRadioButton  *m_pRadTorsion;
    wxRadioButton  *m_pRadConstantColor;

    wxStaticText    *m_pLblMinLength;
    wxStaticText    *m_pLblMaxLength;
    wxStaticText    *m_pLblSubsampling;
    wxStaticText    *m_pColorModeText;
    wxStaticText    *m_pLblColoring;
    wxStaticText    *m_pLblThickness;
};

#endif /* FIBERSGROUP_H_ */
