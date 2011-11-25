#ifndef DATASETINFO_H_
#define DATASETINFO_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/treectrl.h>
#include <wx/tglbtn.h>
#include "DatasetHelper.h"

#include "../misc/IsoSurface/TriangleMesh.h"
#include "../misc/Algorithms/Helper.h"
#include "../gui/SceneObject.h"
#include "../gui/MyListCtrl.h"
#include <GL/glew.h>

union converterByteINT16 
{
    wxUint8 b[2];
    wxUint16 i;
};

union converterByteINT32 
{
    wxUint8 b[4];
    wxUint32 i;
};

union converterByteFloat 
{
    wxUint8 b[4];
    float f;
};

class DatasetHelper;
class TriangleMesh;
class MainFrame;

class DatasetInfo : public SceneObject
{
public:
    DatasetInfo( DatasetHelper* datasetHelper );
    virtual ~DatasetInfo(){};
    
    virtual void createPropertiesSizer(PropertiesWindow *parent); 
    virtual void updatePropertiesSizer();

    virtual bool   load( wxString filename )=0;
    virtual bool   save( wxString filename )const{ return false; }
    virtual void   draw()=0;
    virtual void   clean()=0;
    virtual void   smooth()=0;
    virtual void   flipAxis( AxisType i_axe )=0;
    virtual void   activateLIC()=0;
    virtual GLuint getGLuint()=0;


    wxString getName()                           { return m_name;               };
    wxString getPath()                           { return m_fullPath;           };
    void     setName(wxString name)              { m_name = name;               };
    int      getType()                           { return m_type;               };
    void     setType(int type)                   { m_type = type;               };
    float    getHighestValue()                   { return m_highest_value;      };
    void     setHighestValue(float value)        { m_highest_value = value;     };
    float    getThreshold()                      { return m_threshold;          };
    float    getOldMax()                         { return m_oldMax;             };
    void     setOldMax(float v)                  { m_oldMax = v;                };
    void     setNewMax(float v)                  { m_newMax = v;                };
    float    getNewMax()                         { return m_newMax;             };
	long	 getListCtrlItemId()				 { return m_itemId;				};
	void	 setListCtrlItemId(long value)		 { m_itemId = value;			};
    void     setThreshold(float value)           { m_threshold = value;         };
    float    getAlpha()                          { return m_alpha;              };
    void     setAlpha(float v)                   { m_alpha = v;                 };
    float    getBrightness()                     { return m_brightness;         };
    void     setBrightness( float i_brightness ) { m_brightness = i_brightness; };

    int      getLength()                         { return m_length;   };
    int      getBands()                          { return m_bands;    };
    int      getFrames()                         { return m_frames;   };
    int      getRows()                           { return m_rows;     };
    int      getColumns()                        { return m_columns;  };

    wxString getRpn()                            { return m_repn; };

    bool     toggleShow()                        { m_show        = ! m_show;          return m_show;        };
    bool     toggleShowFS()                      { m_showFS      = ! m_showFS;        return m_showFS;      };
    bool     toggleUseTex()                      { m_useTex      = ! m_useTex;        return m_useTex;      };
    bool     toggleUseLIC()                      { m_useLIC      = ! m_useLIC;        return m_useLIC;      };

    void     setShow       ( bool i_show   )     { m_show        = i_show;   };
    void     setShowFS     ( bool i_show   )     { m_showFS      = i_show;   };
    void     setuseTex     ( bool i_useTex )     { m_useTex      = i_useTex; };
    bool     getShow()                           { return m_show;            };
    bool     getShowFS()                         { return m_showFS;          };
    bool     getUseTex()                         { return m_useTex;          };
    bool     getUseLIC()                         { return m_useLIC;          };
    bool     getIsGlyph()                        { return m_isGlyph;         };

    void     setGLuint( GLuint value )           { m_GLuint = value; };
    void     setColor ( wxColour color )         { m_color  = color; };
    wxColour getColor()                          { return m_color;   };

    // Trianglemesh
    TriangleMesh* m_tMesh;

    wxTextCtrl      *m_ptxtName;
    MySlider        *m_psliderThresholdIntensity;
    MySlider        *m_psliderOpacity;
    wxToggleButton  *m_ptoggleVisibility;
    wxToggleButton  *m_ptoggleFiltering;
    wxBitmapButton  *m_pbtnDelete;
    wxBitmapButton  *m_pbtnUp;
    wxBitmapButton  *m_pbtnDown;
    wxButton        *m_pbtnSmoothLoop;
    wxButton        *m_pbtnClean;
    wxButton		*m_pBtnRename;
    wxButton		*m_pBtnFlipX;
    wxButton		*m_pBtnFlipY;
    wxButton		*m_pBtnFlipZ;
    wxToggleButton  *m_ptoggleLIC;
	wxStaticText	*m_pIntensityText;
	wxStaticText	*m_pOpacityText;

    


protected:
    virtual void generateTexture()=0;
    virtual void generateGeometry()=0;
    virtual void initializeBuffer()=0;

    DatasetHelper* m_dh;

    int         m_length;
    int         m_bands;
    int         m_frames;
    int         m_rows;
    int         m_columns;
    int         m_type;
    wxString    m_repn;
    bool        m_isLoaded;
    float       m_highest_value;
    wxString    m_name;
    wxString    m_fullPath;
    float       m_threshold;
    float       m_alpha;
    float       m_brightness;
    float       m_oldMax;
    float       m_newMax;
	long		m_itemId;

    wxColour    m_color;
    GLuint      m_GLuint;

    bool        m_show;
    bool        m_showFS;       // Show front sector for meshs.
    bool        m_useTex;        // Color mesh with textures loaded.
    
    // If false use colormap on threshold value.
    bool        m_isGlyph;
    bool        licCalculated;
    bool        m_useLIC;
    GLuint*     m_bufferObjects;
};

#endif /*DATASETINFO_H_*/
