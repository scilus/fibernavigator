#ifndef RTTRACKINGHELPER_H_
#define RTTRACKINGHELPER_H_

class RTTrackingHelper
{
public:
    ~RTTrackingHelper();

    static RTTrackingHelper * getInstance();

    bool isFileSelected() const { return m_isFileSelected; }
    bool isRandomSeeds() const  { return m_isRandomSeeds; }
    bool isRTTReady() const     { return m_isRTTReady; }
    bool isRTTDirty() const     { return m_isRTTDirty; }
    bool isRTTActive() const    { return m_isRTTActive; }
    bool isTensorsInterpolated() const  { return m_interpolateTensors; }

    void setFileSelected( bool selected )   { m_isFileSelected = selected; }
    void setRandomSeeds( bool random )      { m_isRandomSeeds = random; }
    void setRTTReady( bool ready )          { m_isRTTReady = ready; }
    void setRTTDirty( bool dirty )          { m_isRTTDirty = dirty; }
    void setRTTActive( bool active )        { m_isRTTActive = active; }

    bool toggleInterpolateTensors() { return m_interpolateTensors = !m_interpolateTensors; }
    bool toggleRandomSeeds()        { return m_isRandomSeeds = !m_isRandomSeeds; }
    bool toggleRTTReady()           { return m_isRTTReady = !m_isRTTReady; }

protected:
    RTTrackingHelper(void);

private:
    RTTrackingHelper( const RTTrackingHelper & );
    RTTrackingHelper &operator=( const RTTrackingHelper &);

private:
    static RTTrackingHelper * m_pInstance;

    bool m_isRTTReady;
    bool m_isRTTDirty;
    bool m_isRTTActive;
    bool m_isRandomSeeds;
    bool m_interpolateTensors;
    bool m_isFileSelected;
};

#endif //RTTRACKINGHELPER_H_
