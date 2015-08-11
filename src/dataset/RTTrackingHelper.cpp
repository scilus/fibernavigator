#include "RTTrackingHelper.h"

#include <stdlib.h>

RTTrackingHelper * RTTrackingHelper::m_pInstance = NULL;

RTTrackingHelper::RTTrackingHelper()
:   m_interpolateTensors( false ),
    m_isFileSelected( false ),
    m_isShellSeeds( false ),
	m_isSeedMap( false ),
    m_isInitSeed( false ),
    m_isRTTActive( false ),
    m_isRTTDirty( false ),
    m_isRTTReady( false ),
    m_isMagnetOn( false ),
    m_isTrackActionPlaying( false ),
    m_isPaused( true ),
	m_isSeedFromfMRI ( false ),
    m_isTractoDrivenRSN( false ),
    m_isGMallowed( false ),
	m_id( 0 )
{
}

RTTrackingHelper * RTTrackingHelper::getInstance()
{
    if( NULL == m_pInstance )
    {
        m_pInstance = new RTTrackingHelper();
    }
    return m_pInstance;
}


RTTrackingHelper::~RTTrackingHelper()
{
    m_pInstance = NULL;
}
