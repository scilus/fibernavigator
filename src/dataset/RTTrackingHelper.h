#ifndef RTTRACKINGHELPER_H_
#define RTTRACKINGHELPER_H_

#include <wx/checkbox.h>
#include <wx/grid.h>
#include <wx/tglbtn.h>
#include <wx/treectrl.h>

#include <wx/scrolwin.h>
#include <wx/statline.h>
#include "../gui/ListCtrl.h"

class RTTrackingHelper
{
public:
    ~RTTrackingHelper();

    static RTTrackingHelper * getInstance();

    bool isFileSelected() const { return m_isFileSelected; }
    bool isShellSeeds() const  { return m_isShellSeeds; }
	bool isSeedMap() const { return m_isSeedMap; }
    bool isInitSeed() const { return m_isInitSeed; }
    bool isMagnetOn() const { return m_isMagnetOn; }
    bool isRTTReady() const     { return m_isRTTReady; }
    bool isRTTDirty() const     { return m_isRTTDirty; }
    bool isRTTActive() const    { return m_isRTTActive; }
    bool isTensorsInterpolated() const  { return m_interpolateTensors; }
	bool isSeedFromfMRI() const { return m_isSeedFromfMRI; }
    bool isTractoDrivenRSN() const { return m_isTractoDrivenRSN; }
    bool isGMAllowed() const { return m_isGMallowed; }
    bool isSrcAlpha() const { return m_isSrcAlpha; }
    bool isRandomInit() const { return m_isRandomInit; }

    void setFileSelected( bool selected )     { m_isFileSelected = selected; }
    void setShellSeeds( bool shell )          { m_isShellSeeds = shell; }
    void setRTTReady( bool ready )            { m_isRTTReady = ready; }
    void setRTTDirty( bool dirty )            { m_isRTTDirty = dirty; }
    void setRTTActive( bool active )          { m_isRTTActive = active; }
	void setSeedFromfMRI( bool seedFromfMRI ) { m_isSeedFromfMRI = seedFromfMRI; }
    void setMaximaFlip( Vector flip)          { flippedAxes.x = flip.x; flippedAxes.y = flip.y; flippedAxes.z = flip.z; }
    Vector getMaximaFlip()          { return flippedAxes; }

    bool toggleInterpolateTensors() { return m_interpolateTensors = !m_interpolateTensors; }
    bool toggleShellSeeds()         { return m_isShellSeeds = !m_isShellSeeds; }
	bool toggleSeedMap()            { return m_isSeedMap = !m_isSeedMap; }
    bool toggleGMmap()              { return m_isGMallowed = !m_isGMallowed; }
    bool toggleInitSeed()           { return m_isInitSeed = !m_isInitSeed; }
    bool toggleRTTReady()           { return m_isRTTReady = !m_isRTTReady; }
    bool toggleMagnet()             { return m_isMagnetOn = !m_isMagnetOn; }
    bool toggleSrcAlpha()           { return m_isSrcAlpha = !m_isSrcAlpha; }
    bool toggleRandomInit()         { return m_isRandomInit = !m_isRandomInit; }
    
    bool togglePlayStop()             { return m_isPaused = !m_isPaused; }
    bool isTrackActionPlaying() const { return m_isTrackActionPlaying; }
    bool isTrackActionPaused() const { return m_isPaused;}
    void setTrackAction(bool enabled) {m_isTrackActionPlaying = enabled;}
    void setTrackActionPause(bool paused) {m_isPaused = paused;}
	void setShellSeed(bool enabled) {m_isShellSeeds = enabled;}
	void setSeedMap(bool enabled) {m_isSeedMap = enabled;}
    void setGMmap(bool enabled) {m_isGMallowed = enabled;}

	int generateId() {return (++m_id);}
    bool toogleTractoDrivenRSN()          { return m_isTractoDrivenRSN = !m_isTractoDrivenRSN; }
    void setEnableTractoRSN(){ m_pBtnToggleEnableRSN->Enable ( true ); } 

    GLuint getBufferID() { return m_bufferID; }
    void setBufferID(GLuint id) { m_bufferID = id;}

	wxSlider            *m_pSliderAxisSeedNb;
	wxTextCtrl          *m_pTxtTotalSeedNbBox;
	wxTextCtrl          *m_pTxtAxisSeedNbBox;
    wxToggleButton      *m_pBtnToggleEnableRSN;
	

protected:
    RTTrackingHelper(void);

private:
    RTTrackingHelper( const RTTrackingHelper & );
    RTTrackingHelper &operator=( const RTTrackingHelper &);

private:
    static RTTrackingHelper * m_pInstance;

    bool m_interpolateTensors;
    bool m_isFileSelected;
    bool m_isShellSeeds;
	bool m_isSeedMap;
    bool m_isInitSeed;
    bool m_isRTTActive;
    bool m_isRTTDirty;
    bool m_isRTTReady;
    bool m_isMagnetOn;
    bool m_isTrackActionPlaying;
    bool m_isPaused;
	bool m_isSeedFromfMRI;
    bool m_isTractoDrivenRSN;
    bool m_isGMallowed;
	int m_id;
    bool m_isSrcAlpha;
    bool m_isRandomInit;
    Vector flippedAxes;
    GLuint m_bufferID;


};

#endif //RTTRACKINGHELPER_H_
