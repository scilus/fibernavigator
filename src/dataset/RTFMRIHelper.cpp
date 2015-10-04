#include "RTFMRIHelper.h"

#include <stdlib.h>

RTFMRIHelper * RTFMRIHelper::m_pInstance = NULL;

RTFMRIHelper::RTFMRIHelper()
:   m_isFileSelected( false ),
    m_isRTFMRIActive( false ),
    m_isRTFMRIDirty( false ),
    m_isRTFMRIReady( false ),
	m_isSeedFromfMRI( false )
{
}

RTFMRIHelper * RTFMRIHelper::getInstance()
{
    if( NULL == m_pInstance )
    {
        m_pInstance = new RTFMRIHelper();
    }
    return m_pInstance;
}


RTFMRIHelper::~RTFMRIHelper()
{
    m_pInstance = NULL;
}
