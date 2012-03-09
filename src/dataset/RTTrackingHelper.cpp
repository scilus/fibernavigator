#include "RTTrackingHelper.h"

#include <stdlib.h>

RTTrackingHelper * RTTrackingHelper::m_pInstance = NULL;

RTTrackingHelper::RTTrackingHelper()
:   m_interpolateTensors( false ),
    m_isFileSelected( false ),
    m_isRandomSeeds( false ),
    m_isRTTActive( false ),
    m_isRTTDirty( false ),
    m_isRTTReady( false )
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
