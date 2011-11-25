/*
 *  The FibersGroup class declaration.
 *
 */

#ifndef FIBERSGROUP_H_
#define FIBERSGROUP_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <vector>

#include "DatasetInfo.h"
#include "Fibers.h"

using namespace std;

/**
 * This class represents a group container for sets of fibers.
 * It is useful to load several sets of fibers and can be used to
 * apply a characteristic to all set of fibers that are in the group.
 */
class FibersGroup : public DatasetInfo, public wxTreeItemData
{
public:
    FibersGroup( DatasetHelper *pDatasetHelper );
    // TODO copy constructors: should they be allowed?
    virtual ~FibersGroup();
    
	void	addFibersSet(Fibers* pFibers);
	bool	removeFibersSet(Fibers* pFibers);
	Fibers* getFibersSet(int num);

	int		getFibersCount() { return (int)m_fibersSets.size(); };

	bool	isOneBtnToggled()
	{
		return m_isIntensityToggled && m_isOpacityToggled && m_isMinMaxLengthToggled 
			   && m_isSubsamplingToggled && m_isColorModeToggled;
	}

	void    save( wxString filename );
    void    saveDMRI( wxString filename );

    void    updateGroupFilters();
	void	resetFibersColor();
	void	invertFibers();
	void	useFakeTubes();
	void	useTransparency();
    
    void    resetAllValues();

	void	fibersLocalColoring();
	void	fibersNormalColoring();
	
	void	generateGlobalFiberVolume(std::vector<Anatomy*> vAnatomies);

	void	OnDeleteFibers();
	void	OnToggleVisibleBtn();
	void	OnToggleIntensityBtn();
	void	OnToggleOpacityBtn();
	void	OnToggleMinMaxLengthBtn();
	void	OnToggleSubsamplingBtn();
	void	OnToggleColorModeBtn();
	void	OnToggleLocalColoring();
	void	OnToggleNormalColoring();
	void	OnClickGenerateFiberVolumeBtn();
	void	OnClickApplyBtn();
	void	OnClickCancelBtn();
	
	// Empty derived methods
	bool    load( wxString  filename ) { return false; };
	void    draw()						{};
	void    smooth()					{};
	void	flipAxis( AxisType i_axe )	{};
    void    activateLIC()				{};
    void    clean()						{};
    void    drawVectors()				{};
    void    generateTexture()			{};
    void    generateGeometry()			{};
	void    initializeBuffer()			{};
	
	GLuint  getGLuint( )
    {
        return 0;
    };

	virtual void createPropertiesSizer( PropertiesWindow *pParent );
	virtual void updatePropertiesSizer();

private: 
	// Variables
	std::vector<Fibers*> m_fibersSets;

	bool m_isIntensityToggled;
	bool m_isOpacityToggled;
	bool m_isMinMaxLengthToggled;
	bool m_isSubsamplingToggled;
	bool m_isColorModeToggled;

	bool m_isNormalColoringStateChanged;
	bool m_isLocalColoringStateChanged;

	string intToString( const int number );
    
    // GUI members
	wxToggleButton *m_ptoggleIntensity;
	wxToggleButton *m_ptoggleOpacity;
	wxToggleButton *m_ptoggleMinMaxLength;
	wxToggleButton *m_ptoggleSubsampling;
	wxToggleButton *m_ptoggleColorMode;
    wxToggleButton *m_pToggleLocalColoring;
    wxToggleButton *m_pToggleNormalColoring;

	wxButton	   *m_pApplyBtn;
	wxButton	   *m_pCancelBtn;

	wxButton       *m_pGeneratesFibersDensityVolume;
    wxSlider       *m_pSliderFibersFilterMin;
    wxSlider       *m_pSliderFibersFilterMax;
    wxSlider       *m_pSliderFibersSampling;
	wxRadioButton  *m_pRadioNormalColoring;
    wxRadioButton  *m_pRadioDistanceAnchoring;
    wxRadioButton  *m_pRadioMinDistanceAnchoring;
    wxRadioButton  *m_pRadioCurvature;
    wxRadioButton  *m_pRadioTorsion;

	wxStaticText	*m_pMinLengthText;
	wxStaticText	*m_pMaxLengthText;
	wxStaticText	*m_pSubsamplingText;
	wxStaticText	*m_pColorModeText;
	wxStaticText	*m_pColoringText;
};

#endif /* FIBERSGROUP_H_ */
