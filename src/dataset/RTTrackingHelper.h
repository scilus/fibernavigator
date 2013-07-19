#ifndef RTTRACKINGHELPER_H_
#define RTTRACKINGHELPER_H_

class RTTrackingHelper
{
public:
    ~RTTrackingHelper();

    static RTTrackingHelper * getInstance();

    bool isFileSelected() const { return m_isFileSelected; }
    bool isShellSeeds() const  { return m_isShellSeeds; }
	bool isSeedMap() const { return m_isSeedMap; }
    bool isRTTReady() const     { return m_isRTTReady; }
    bool isRTTDirty() const     { return m_isRTTDirty; }
    bool isRTTActive() const    { return m_isRTTActive; }
    bool isTensorsInterpolated() const  { return m_interpolateTensors; }

    void setFileSelected( bool selected )   { m_isFileSelected = selected; }
    void setShellSeeds( bool shell )        { m_isShellSeeds = shell; }
    void setRTTReady( bool ready )          { m_isRTTReady = ready; }
    void setRTTDirty( bool dirty )          { m_isRTTDirty = dirty; }
    void setRTTActive( bool active )        { m_isRTTActive = active; }

    bool toggleInterpolateTensors() { return m_interpolateTensors = !m_interpolateTensors; }
    bool toggleShellSeeds()        { return m_isShellSeeds = !m_isShellSeeds; }
	bool toggleSeedMap()           { return m_isSeedMap = !m_isSeedMap; }
    bool toggleRTTReady()           { return m_isRTTReady = !m_isRTTReady; }
    
    bool togglePlayStop()             { return m_isPaused = !m_isPaused; }
    bool isTrackActionPlaying() const { return m_isTrackActionPlaying; }
    bool isTrackActionPaused() const { return m_isPaused;}
    void setTrackAction(bool enabled) {m_isTrackActionPlaying = enabled;}
    void setTrackActionPause(bool paused) {m_isPaused = paused;}
	

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
    bool m_isRTTActive;
    bool m_isRTTDirty;
    bool m_isRTTReady;
    bool m_isTrackActionPlaying;
    bool m_isPaused;

};

#endif //RTTRACKINGHELPER_H_
