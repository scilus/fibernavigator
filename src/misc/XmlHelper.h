#ifndef XMLHELPER_H_
#define XMLHELPER_H_

#include <wx/string.h>

class wxXmlNode;
class wxXmlProperty;

wxXmlNode *     getXmlNodeByName(     const wxString name, wxXmlNode * pNode );
//wxXmlProperty * getXmlPropertyByName( const wxString name, wxXmlNode * pNode );

#endif //XMLHELPER_H_