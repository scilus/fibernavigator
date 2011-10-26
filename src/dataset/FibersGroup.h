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
	Fibers* getFibersSet(int num);

	int		getFibersCount() { return m_fibersCount; };

    void    updateGroupFilters();

	void	OnToggleIntensityBtn();
	void	OnToggleOpacityBtn();
	void	OnToggleMinMaxLengthBtn();
	void	OnToggleSubsamplingBtn();
	void	OnToggleColorModeBtn();
	void	OnClickApplyBtn();
	void	OnClickCancelBtn();
	
	// Empty derived methods
	bool    load( wxString  filename ) { return false; };
	void    draw()			   {};
	void    smooth()           {};
    void    activateLIC()      {};
    void    clean()            {};
    void    drawVectors()      {};
    void    generateTexture()  {};
    void    generateGeometry() {};
	void    initializeBuffer() {};
	
	GLuint  getGLuint( )
    {
        return 0;
    };

	virtual void createPropertiesSizer( PropertiesWindow *pParent );
	virtual void updatePropertiesSizer();

private:    
    // Variables
	std::vector<Fibers*> m_fibersSets;

	int	 m_fibersCount;

	bool m_isIntensityToggled;
	bool m_isOpacityToggled;
	bool m_isMinMaxLengthToggled;
	bool m_isSubsamplingToggled;
	bool m_isColorModeToggled;
    
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
};

#endif /* FIBERSGROUP_H_ */
