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

wxString wxStrFormat( int val, wxString precision /*= wxT( "" )*/ )
{
    return wxString::Format( wxT( "%" ) + precision + wxT( "d" ), val );
}

wxString wxStrFormat( double val, wxString precision /*= wxT( "" )*/ )
{
    return wxString::Format( wxT( "%" ) + precision + wxT( "f" ), val );
}

bool parseXmlBoolString( wxString &str )
{
    return str == wxT("yes") ? true : false;
}
