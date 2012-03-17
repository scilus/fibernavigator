#ifndef DATASETINFO_H_
#define DATASETINFO_H_

#include "../misc/Algorithms/Helper.h"
#include "../gui/SceneObject.h"

#include <GL/glew.h>
#include <wx/colour.h>
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

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

class MySlider;
class PropertiesWindow;
class TriangleMesh;
class wxBitmapButton;
class wxButton;
class wxStaticText;
class wxTextCtrl;
class wxToggleButton;
class wxXmlNode;
class wxXmlProperty;

class DatasetInfo : public SceneObject
{
public:
    DatasetInfo();
    virtual ~DatasetInfo();

    // Pure Virtual functions
    virtual void   draw()                       = 0;
    virtual void   clean()                      = 0;
    virtual void   smooth()                     = 0;
    virtual void   flipAxis( AxisType i_axe )   = 0;
    virtual GLuint getGLuint()                  = 0;

    // Virtual functions
    virtual void createPropertiesSizer(PropertiesWindow *parent); 
    virtual void updatePropertiesSizer();
    virtual bool save( wxString filename ) const { return false; }
    virtual bool save( wxXmlNode *node ) const;

    // Functions
    wxString getName() const                     { return m_name;               };
    wxString getPath() const                     { return m_fullPath;           };
    void     setName(wxString name)              { m_name = name;               };
    int      getType() const                     { return m_type;               };
    void     setType(int type)                   { m_type = type;               };
    float    getHighestValue() const             { return m_highest_value;      };
    void     setHighestValue(float value)        { m_highest_value = value;     };
    float    getThreshold() const                { return m_threshold;          };
    float    getOldMax() const                   { return m_oldMax;             };
    void     setOldMax(float v)                  { m_oldMax = v;                };
    void     setNewMax(float v)                  { m_newMax = v;                };
    float    getNewMax() const                   { return m_newMax;             };
    void     setThreshold(float value)           { m_threshold = value;         };
    float    getAlpha() const                    { return m_alpha;              };
    void     setAlpha(float v)                   { m_alpha = v;                 };
    float    getBrightness() const               { return m_brightness;         };
    void     setBrightness( float i_brightness ) { m_brightness = i_brightness; };

    int      getLength() const                   { return m_length;   };
    int      getBands() const                    { return m_bands;    };
    int      getFrames() const                   { return m_frames;   };
    int      getRows() const                     { return m_rows;     };
    int      getColumns() const                  { return m_columns;  };

    wxString getRpn() const                      { return m_repn; };

    bool     toggleShow()                        { m_show        = !m_show;          return m_show;        };
    bool     toggleShowFS()                      { m_showFS      = !m_showFS;        return m_showFS;      };
    bool     toggleUseTex()                      { m_useTex      = !m_useTex;        return m_useTex;      };

    void     setShow       ( bool i_show   )     { m_show        = i_show;   };
    void     setShowFS     ( bool i_show   )     { m_showFS      = i_show;   };
    void     setUseTex     ( bool i_useTex )     { m_useTex      = i_useTex; };
    bool     getShow() const                     { return m_show;            };
    bool     getShowFS() const                   { return m_showFS;          };
    bool     getUseTex() const                   { return m_useTex;          };
    bool     getIsGlyph() const                  { return m_isGlyph;         };

    void     setGLuint( GLuint value )           { m_GLuint = value; };
    void     setColor ( wxColour color )         { m_color  = color; };
    wxColour getColor() const                    { return m_color;   };

    int getVoxelSizeX() const                    { return m_voxelSizeX; }
    int getVoxelSizeY() const                    { return m_voxelSizeY; }
    int getVoxelSizeZ() const                    { return m_voxelSizeZ; }

public:
    // Trianglemesh
    TriangleMesh    *m_tMesh;

    wxTextCtrl      *m_ptxtName;
    MySlider        *m_psliderThresholdIntensity;
    MySlider        *m_psliderOpacity;
    wxToggleButton  *m_pToggleVisibility;
    wxToggleButton  *m_pToggleFiltering;
    wxBitmapButton  *m_pbtnDelete;
    wxBitmapButton  *m_pbtnUp;
    wxBitmapButton  *m_pbtnDown;
    wxButton        *m_pbtnSmoothLoop;
    wxButton        *m_pbtnClean;
    wxButton        *m_pBtnRename;
    wxToggleButton  *m_pBtnFlipX;
    wxToggleButton  *m_pBtnFlipY;
    wxToggleButton  *m_pBtnFlipZ;
	wxStaticText	*m_pIntensityText;
	wxStaticText	*m_pOpacityText;

protected:
    virtual void generateTexture()  = 0;
    virtual void generateGeometry() = 0;
    virtual void initializeBuffer() = 0;

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

    wxColour    m_color;
    GLuint      m_GLuint;

    bool        m_show;
    bool        m_showFS;       // Show front sector for meshs.
    bool        m_useTex;        // Color mesh with textures loaded.
    
    // If false use colormap on threshold value.
    bool        m_isGlyph;
    GLuint*     m_bufferObjects;

    float       m_voxelSizeX;
    float       m_voxelSizeY;
    float       m_voxelSizeZ;
};

#endif /*DATASETINFO_H_*/
