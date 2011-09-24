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
class FibersGroup : public DatasetInfo
{
public:
    FibersGroup( DatasetHelper *pDatasetHelper );
    // TODO copy constructors: should they be allowed?
    virtual ~FibersGroup();
    
    void    updateGroupFilters();
	
	virtual void createPropertiesSizer( PropertiesWindow *pParent );
	virtual void updatePropertiesSizer();

private:    
    // Variables
	std::vector<Fibers*> m_fibersSets;
    bool				 m_isInitialized;
    
    // GUI members
    wxButton       *m_pGeneratesFibersDensityVolume;
    wxSlider       *m_pSliderFibersFilterMin;
    wxSlider       *m_pSliderFibersFilterMax;
    wxSlider       *m_pSliderFibersSampling;
    wxToggleButton *m_pToggleLocalColoring;
    wxToggleButton *m_pToggleNormalColoring;
    wxRadioButton  *m_pRadioNormalColoring;
    wxRadioButton  *m_pRadioDistanceAnchoring;
    wxRadioButton  *m_pRadioMinDistanceAnchoring;
    wxRadioButton  *m_pRadioCurvature;
    wxRadioButton  *m_pRadioTorsion;
};

#endif /* FIBERSGROUP_H_ */
