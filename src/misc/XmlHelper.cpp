#include "XmlHelper.h"

#include <wx/xml/xml.h>

wxXmlNode * getXmlNodeByName( const wxString name, wxXmlNode * pNode )
{
    assert( pNode != NULL );

    wxXmlNode *pChild = pNode->GetChildren();
    while( NULL != pChild && name != pChild->GetName() )
    {
        pChild = pChild->GetNext();
    }
    return pChild;
}

//////////////////////////////////////////////////////////////////////////

/*wxXmlProperty * getXmlPropertyByName( const wxString name, wxXmlNode * pNode )
{
    assert( NULL != pNode );

    wxXmlProperty *pProp = pNode->GetProperties();
    while( NULL != pProp && name != pProp->GetName() )
    {
        pProp = pProp->GetNext();
    }
    return pProp;
}*/
