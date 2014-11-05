#ifndef RTFMRIHELPER_H_
#define RTFMRIHELPER_H_

class RTFMRIHelper
{
public:
    ~RTFMRIHelper();

    static RTFMRIHelper * getInstance();

    bool isFileSelected() const { return m_isFileSelected; }
    bool isRTFMRIReady() const     { return m_isRTFMRIReady; }
    bool isRTFMRIDirty() const     { return m_isRTFMRIDirty; }
    bool isRTFMRIActive() const    { return m_isRTFMRIActive; }
	bool isSeedFromfMRI() const { return m_isSeedFromfMRI; }

    void setFileSelected( bool selected )   { m_isFileSelected = selected; }
    void setRTFMRIReady( bool ready )          { m_isRTFMRIReady = ready; }
    void setRTFMRIDirty( bool dirty )          { m_isRTFMRIDirty = dirty; }
    void setRTFMRIActive( bool active )        { m_isRTFMRIActive = active; }
	void setSeedFromfMRI( bool seedFromfMRI )  { m_isSeedFromfMRI = seedFromfMRI; }

    bool toggleRTFMRIReady()           { return m_isRTFMRIReady = !m_isRTFMRIReady; }
	bool toogleSeedFromfMRI()          { return m_isSeedFromfMRI = !m_isSeedFromfMRI; }
    
protected:
    RTFMRIHelper(void);

private:
    RTFMRIHelper( const RTFMRIHelper & );
    RTFMRIHelper &operator=( const RTFMRIHelper &);

private:
    static RTFMRIHelper * m_pInstance;

    bool m_isFileSelected;
    bool m_isRTFMRIActive;
    bool m_isRTFMRIDirty;
    bool m_isRTFMRIReady;
	bool m_isSeedFromfMRI;

};

#endif //RTFMRIHELPER_H_