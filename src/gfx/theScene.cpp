
#include "theScene.h"
#include "../gui/myListCtrl.h"
#include "../dataset/splinePoint.h"
#include "../dataset/fibers.h"
#include "../dataset/surface.h"
#include "../gui/selectionBox.h"
#include "../dataset/AnatomyHelper.h"
#include "../dataset/Anatomy.h"
#include "../misc/IsoSurface/CIsoSurface.h"

#ifdef CG_GLYPHS
#include "../gfx/ImplicitSurfaceGlyphs/TensorStorageHelper.hh"
#include <limits>
#endif //CG_GLYPHS

TheScene::TheScene(DatasetHelper* dh)
:   m_dh(dh),
    m_mainGLContext(0)
#ifdef CG_GLYPHS
    ,
    m_old_xSlice( -1 ),
    m_old_ySlice( -1 ),
    m_old_zSlice( -1 ),
    m_myGlyphs( 0 ),
    m_glyphsInitialized( false ),
    m_old_nbGlyphPositions( 0 )
#endif //CG_GLYPHS
{
	m_dh->anatomyHelper = new AnatomyHelper(m_dh);
}

TheScene::~TheScene()
{
	m_dh->printDebug(_T("execute scene destructor"), 0);

#ifndef __WXMAC__
    // on mac, this is just a pointer to the original object that is deleted with the widgets
	if (m_mainGLContext) delete m_mainGLContext;
#endif
	m_dh->printDebug(_T("scene destructor done"), 0);

#ifdef CG_GLYPHS
    if( m_myGlyphs )
    {
        delete m_myGlyphs;
        m_myGlyphs = 0;
    }
#endif //CG_GLYPHS

}

void TheScene::initGL(int view)
{
	try
	{
		GLenum err = glewInit();
		if (GLEW_OK != err)
		{
			/* Problem: glewInit failed, something is seriously wrong. */
			m_dh->printDebug(_T("Error: ") + wxString::FromAscii((char*)glewGetErrorString(err)), 2);
			exit(false);
		}
		if (view == mainView)
		{
			m_dh->printDebug(_T("Status: Using GLEW ") +  wxString::FromAscii((char*)glewGetString(GLEW_VERSION)), 1);


			wxString vendor;
			wxString renderer;
			vendor = wxString::FromAscii((char*)glGetString(GL_VENDOR));
			renderer = wxString::FromAscii((char*)glGetString(GL_RENDERER));

			if ( renderer.Contains( _T("GeForce 6")) )
			{
				m_dh->geforceLevel = 6;
			}
			else if ( renderer.Contains( _T("GeForce 7")) )
			{
				m_dh->geforceLevel = 7;
			}
			else if ( renderer.Contains( _T("GeForce 8")) || renderer.Contains( _T("GeForce GTX 2")))
			{
				m_dh->geforceLevel = 8;
			}


			m_dh->printDebug(vendor + _T(" ") + renderer, 1);

			if( !glewIsSupported("GL_ARB_shader_objects") )
			{
				printf("*** ERROR no support for shader objects found.\n");
				printf("*** Please check your OpenGL installation...exiting.\n");
				exit(false);
			}
		}
		glEnable(GL_DEPTH_TEST);

		if (!m_dh->m_texAssigned)
		{
			m_dh->shaderHelper = new ShaderHelper(m_dh);
			m_dh->m_texAssigned = true;
		}

		//float maxLength = (float)wxMax(m_dh->columns, wxMax(m_dh->rows, m_dh->frames));
		//float view1 = maxLength;
		float view1 = 200;
		glClearColor(1.0, 1.0, 1.0, 0.0);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho( 0, view1, 0, view1, -3000, 3000);
	}
	catch (...)
	{
		if ( m_dh->GLError() )
		    m_dh->printGLError(wxT("init"));
	}
}

void TheScene::bindTextures()
{
	glEnable(GL_TEXTURE_3D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	int c = 0;

	for (int i = 0 ; i < m_dh->mainFrame->m_listCtrl->GetItemCount() ; ++i)
	{
		DatasetInfo* info = (DatasetInfo*)m_dh->mainFrame->m_listCtrl->GetItemData(i);
		if (info->getType() < Mesh_)
		{
			glActiveTexture(GL_TEXTURE0 + c);
			glBindTexture(GL_TEXTURE_3D, info->getGLuint());
			if(info->getShowFS())
			{
				glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
			else
			{
				glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			}
			if ( ++c == 10 )
            {
                printf("reached 10 textures\n");
                break;
            }
		}

	}
	if ( m_dh->GLError() )
	    m_dh->printGLError(wxT("bind textures"));
}

void TheScene::renderScene()
{
	if ( m_dh->mainFrame->m_listCtrl->GetItemCount() == 0 ) return;

	m_dh->shaderHelper->initializeArrays();

	renderSlizes();

	if ( m_dh->surface_loaded )
	    renderSplineSurface();

	if ( m_dh->pointMode )
		drawPoints();

	if ( m_dh->fibers_loaded )
	{
		if  ( m_dh->useFakeTubes )
			renderFakeTubes();
		else
			renderFibers();
	}

	if ( m_dh->mesh_loaded )
	    renderMesh();

	if ( m_dh->vectors_loaded )
	    drawVectors();

	if ( m_dh->fibers_loaded && m_dh->showBoxes )
		drawSelectionBoxes();

	if ( m_dh->showColorMapLegend )
		drawColorMapLegend();

#ifdef CG_GLYPHS
	if( m_dh->showGlyphs )
	{
	    //drawSingleGlyph();
	    drawGlyphsOnSurface();
	}
#endif //CG_GLYPHS

	if ( m_dh->GLError() )
	    m_dh->printGLError(wxT("render scene"));
}

void TheScene::renderSlizes()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	if (m_dh->blendAlpha)
		glDisable(GL_ALPHA_TEST);
		//glDisable(GL_BLEND);
	else
		glEnable(GL_ALPHA_TEST);
		//glEnable(GL_BLEND);

	glAlphaFunc(GL_GREATER, 0.001f); // adjust your preferred threshold here

	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	bindTextures();
	m_dh->shaderHelper->m_textureShader->bind();
	m_dh->shaderHelper->setTextureShaderVars();
	m_dh->shaderHelper->m_textureShader->setUniInt("useColorMap", m_dh->colorMap);

	m_dh->anatomyHelper->renderMain();

	glDisable(GL_BLEND);

	m_dh->shaderHelper->m_textureShader->release();

	if (m_dh->showCrosshair)
		m_dh->anatomyHelper->renderCrosshair();

	if ( m_dh->GLError() )
	    m_dh->printGLError(wxT("render slizes"));

	glPopAttrib();
}

void TheScene::renderSplineSurface()
{
	for (int i = 0 ; i < m_dh->mainFrame->m_listCtrl->GetItemCount() ; ++i)
	{
		DatasetInfo* info = (DatasetInfo*)m_dh->mainFrame->m_listCtrl->GetItemData(i);
		if (info->getType() == Surface_ && info->getShow())
		{
			glPushAttrib(GL_ALL_ATTRIB_BITS);

			if (m_dh->pointMode)
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			bindTextures();

			lightsOn();

			m_dh->shaderHelper->m_splineSurfShader->bind();
			m_dh->shaderHelper->setSplineSurfaceShaderVars();

			wxColor c = info->getColor();
			glColor3f((float)c.Red()/255.0, (float)c.Green()/255.0, (float)c.Blue()/255.0);
			m_dh->shaderHelper->m_splineSurfShader->setUniInt("useTex", !info->getUseTex());
			m_dh->shaderHelper->m_splineSurfShader->setUniInt("useLic", info->getUseLIC());
			m_dh->shaderHelper->m_splineSurfShader->setUniInt("useColorMap", m_dh->colorMap);

			info->draw();

			m_dh->shaderHelper->m_splineSurfShader->release();

			lightsOff();

			if ( m_dh->GLError() )
			    m_dh->printGLError(wxT("draw surface"));

			glPopAttrib();
		}
	}
}

void TheScene::renderMesh()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	if ( m_dh->lighting )
    {
	    lightsOn();
    }

	bindTextures();

	m_dh->shaderHelper->m_meshShader->bind();
	m_dh->shaderHelper->setMeshShaderVars();

	if (m_dh->pointMode)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	for (int i = 0 ; i < m_dh->mainFrame->m_listCtrl->GetItemCount() ; ++i)
	{
		DatasetInfo* info = (DatasetInfo*)m_dh->mainFrame->m_listCtrl->GetItemData(i);
		if (info->getType() == Mesh_ || info->getType() == IsoSurface_)
		{
			if (info->getShow())
			{
				wxColor c = info->getColor();
                glColor3f((float)c.Red()/255.0, (float)c.Green()/255.0, (float)c.Blue()/255.0);
                m_dh->shaderHelper->m_meshShader->setUniInt("showFS", info->getShowFS());
                m_dh->shaderHelper->m_meshShader->setUniInt("useTex", info->getUseTex());
                m_dh->shaderHelper->m_meshShader->setUniFloat("alpha_", info->getAlpha());
                m_dh->shaderHelper->m_meshShader->setUniInt("useColorMap", m_dh->colorMap);
                m_dh->shaderHelper->m_meshShader->setUniInt("useLic", info->getUseLIC());
                m_dh->shaderHelper->m_meshShader->setUniInt("useCMAP", info->getIsGlyph());
                m_dh->shaderHelper->m_meshShader->setUniInt("lightOn", m_dh->lighting);

                if (m_dh->surface_loaded)
                    m_dh->shaderHelper->m_meshShader->setUniInt("cutAtSurface", true);
                else
                    m_dh->shaderHelper->m_meshShader->setUniInt("cutAtSurface", false);

                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                info->draw();
			}
		}
	}


	std::vector<std::vector<SelectionBox*> > boxes = m_dh->getSelectionBoxes();

	m_dh->shaderHelper->m_meshShader->bind();
	m_dh->shaderHelper->m_meshShader->setUniInt("showFS", true);
	m_dh->shaderHelper->m_meshShader->setUniInt("useTex", false);
	m_dh->shaderHelper->m_meshShader->setUniFloat("alpha_", 1.0);
	m_dh->shaderHelper->m_meshShader->setUniInt("useColorMap",  m_dh->colorMap);
	m_dh->shaderHelper->m_meshShader->setUniInt("useLic", false);
	m_dh->shaderHelper->m_meshShader->setUniInt("useCMAP", false);

	for (unsigned int i = 0 ; i < boxes.size() ; ++i)
	{
		for (unsigned int j = 0 ; j < boxes[i].size() ; ++j)
		{
			glPushAttrib(GL_ALL_ATTRIB_BITS);
			if ( !boxes[i][j]->getIsBox() )
				boxes[i][j]->drawIsoSurface();
			glPopAttrib();
		}
	}

	m_dh->shaderHelper->m_meshShader->release();

	lightsOff();

	if ( m_dh->GLError() )
	    m_dh->printGLError(wxT("draw mesh"));

	glPopAttrib();
}

void TheScene::renderFibers()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	for (int i = 0 ; i < m_dh->mainFrame->m_listCtrl->GetItemCount() ; ++i)
	{
		DatasetInfo* info = (DatasetInfo*)m_dh->mainFrame->m_listCtrl->GetItemData(i);

		if (info->getType() == Fibers_ && info->getShow())
		{
			lightsOff();

			if ( m_dh->lighting )
			{
				lightsOn();
				GLfloat light_position0[] = { 1.0, 1.0, 1.0, 0.0};
				glLightfv (GL_LIGHT0, GL_POSITION, light_position0);

			}
			if ( !info->getUseTex() )
			{
				bindTextures();
				m_dh->shaderHelper->m_fiberShader->bind();
				m_dh->shaderHelper->setFiberShaderVars();
				m_dh->shaderHelper->m_fiberShader->setUniInt("useTex", !info->getUseTex());
				m_dh->shaderHelper->m_fiberShader->setUniInt("useColorMap", m_dh->colorMap);
				m_dh->shaderHelper->m_fiberShader->setUniInt("useOverlay", info->getShowFS());
			}
			if ( m_dh->m_selBoxChanged )
			{
				((Fibers*)info)->updateLinesShown();
				m_dh->m_selBoxChanged = false;
			}
			info->draw();

			m_dh->shaderHelper->m_fiberShader->release();

			lightsOff();
		}
	}

	if ( m_dh->GLError() )
	    m_dh->printGLError(wxT("draw fibers"));

	glPopAttrib();
}

void TheScene::renderFakeTubes()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	for (int i = 0 ; i < m_dh->mainFrame->m_listCtrl->GetItemCount() ; ++i)
	{
		DatasetInfo* info = (DatasetInfo*)m_dh->mainFrame->m_listCtrl->GetItemData(i);

		if (info->getType() == Fibers_ && info->getShow())
		{
			if ( m_dh->m_selBoxChanged )
			{
				((Fibers*)info)->updateLinesShown();
				m_dh->m_selBoxChanged = false;
			}

			m_dh->shaderHelper->m_fakeTubeShader->bind();
			m_dh->shaderHelper->m_fakeTubeShader->setUniInt("globalColor", info->getShowFS());
			m_dh->shaderHelper->m_fakeTubeShader->setUniFloat("dimX", (float)m_dh->mainFrame->m_mainGL->GetSize().x);
			m_dh->shaderHelper->m_fakeTubeShader->setUniFloat("dimY", (float)m_dh->mainFrame->m_mainGL->GetSize().y);

			info->draw();

			m_dh->shaderHelper->m_fakeTubeShader->release();
		}
	}

	if ( m_dh->GLError() )
	    m_dh->printGLError(wxT("draw fake tubes"));

	glPopAttrib();
}

void TheScene::lightsOn()
{
	GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat light_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	GLfloat light_specular[] = { 0.4f, 0.4f, 0.4f, 1.0f };
	GLfloat specref[] = { 0.5, 0.5, 0.5, 0.5};

	Vector3fT v1 = {{0,0,-1}};
	Vector3fT l;
	Vector3fMultMat4(&l, &v1, &m_dh->m_transform);

	GLfloat light_position0[] = { l.s.X, l.s.Y, l.s.Z, 0.0};

	glLightfv (GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv (GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv (GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv (GL_LIGHT0, GL_POSITION, light_position0);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glShadeModel(GL_SMOOTH);

	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specref);
	glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 32);

	if ( m_dh->GLError() )
	    m_dh->printGLError(wxT("setup lights"));
}

void TheScene::lightsOff()
{
	glDisable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);
}

void TheScene::drawSphere(float x, float y, float z, float r)
{
	glPushMatrix();
	glTranslatef(x,y,z);

	GLUquadricObj *quadric = gluNewQuadric();
	gluQuadricNormals(quadric, GLU_SMOOTH);
	gluSphere(quadric, r, 32, 32);
	glPopMatrix();

	if ( m_dh->GLError() )
	    m_dh->printGLError(wxT("draw sphere"));
}

void TheScene::drawSelectionBoxes()
{
	std::vector<std::vector<SelectionBox*> > boxes = m_dh->getSelectionBoxes();
	for (unsigned int i = 0 ; i < boxes.size() ; ++i)
	{
		for (unsigned int j = 0 ; j < boxes[i].size() ; ++j)
		{
			glPushAttrib(GL_ALL_ATTRIB_BITS);

			boxes[i][j]->draw();

			glPopAttrib();
		}
	}

	if ( m_dh->GLError() )
	    m_dh->printGLError(wxT("draw selection boxes"));
}

void TheScene::drawPoints()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	lightsOn();
	m_dh->shaderHelper->m_meshShader->bind();
	m_dh->shaderHelper->setMeshShaderVars();
	m_dh->shaderHelper->m_meshShader->setUniInt("showFS", true);
	m_dh->shaderHelper->m_meshShader->setUniInt("useTex", false);
	m_dh->shaderHelper->m_meshShader->setUniInt("cutAtSurface", false);
	m_dh->shaderHelper->m_meshShader->setUniInt("lightOn", true);

	wxTreeItemId id, childid;
	wxTreeItemIdValue cookie = 0;
	id = m_dh->mainFrame->m_treeWidget->GetFirstChild(m_dh->mainFrame->m_tPointId, cookie);
	while ( id.IsOk() )
	{
		SplinePoint *point = (SplinePoint*)(m_dh->mainFrame->m_treeWidget->GetItemData(id));
		point->draw();

		id = m_dh->mainFrame->m_treeWidget->GetNextChild(m_dh->mainFrame->m_tPointId, cookie);
	}

	lightsOff();
	m_dh->shaderHelper->m_meshShader->release();
	glPopAttrib();

	if ( m_dh->GLError() )
	    m_dh->printGLError(wxT("draw points"));
}

void TheScene::drawColorMapLegend()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	int size = wxMax(wxMax(m_dh->rows, m_dh->columns), m_dh->frames);
	glOrtho( 0, size, 0, size, -3000, 3000);

	m_dh->shaderHelper->m_legendShader->bind();
	m_dh->shaderHelper->m_legendShader->setUniInt("useColorMap", m_dh->colorMap);

	glColor3f(0.0, 0.0, 0.0);
	glLineWidth(5.0);
	glBegin(GL_LINES);
		glTexCoord1f(0.0);
		glVertex3i(size - 60, 10, 2900);
		glTexCoord1f(1.0);
		glVertex3i(size - 20, 10, 2900);
	glEnd();

	m_dh->shaderHelper->m_legendShader->release();

	glLineWidth(1.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_LINES);
		glVertex3i(size - 60, 10, 2900);
		glVertex3i(size - 60, 12, 2900);
		glVertex3i(size - 50, 10, 2900);
		glVertex3i(size - 50, 12, 2900);
		glVertex3i(size - 40, 10, 2900);
		glVertex3i(size - 40, 12, 2900);
		glVertex3i(size - 30, 10, 2900);
		glVertex3i(size - 30, 12, 2900);
		glVertex3i(size - 20, 10, 2900);
		glVertex3i(size - 20, 12, 2900);
	glEnd();

	m_dh->shaderHelper->m_legendShader->release();

	glPopMatrix();
	glPopAttrib();
}

void TheScene::drawVectors()
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable( GL_LINE_SMOOTH);
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST);

    for (int i = 0; i < m_dh->mainFrame->m_listCtrl->GetItemCount(); ++i)
    {
        DatasetInfo* info = (DatasetInfo*) m_dh->mainFrame->m_listCtrl->GetItemData(i);

        if (info->getType() == Vectors_ && info->getShow())
        {
            Anatomy* vecs = (Anatomy*) info;
            glLineWidth(1.0);
            glBegin(GL_LINES);

            float r, g, b, a;

            r = vecs->getColor().Red() / 255.;
            g = vecs->getColor().Green() / 255.;
            b = vecs->getColor().Blue() / 255.;
            a = 1.0;

            float bright = 1.2;
            float dull = 0.7;

            bool topview   = m_dh->quadrant == 2 || m_dh->quadrant == 3 || m_dh->quadrant == 6 || m_dh->quadrant == 7;
            bool leftview  = m_dh->quadrant == 5 || m_dh->quadrant == 6 || m_dh->quadrant == 7 || m_dh->quadrant == 8;
            bool frontview = m_dh->quadrant == 3 || m_dh->quadrant == 4 || m_dh->quadrant == 5 || m_dh->quadrant == 6;

            if (m_dh->showAxial)
            {
                for (int i = 0; i < m_dh->columns; ++i)
                {
                    for (int j = 0; j < m_dh->rows; ++j)
                    {
                        int slize = (int)(m_dh->zSlize * m_dh->columns * m_dh->rows * 3);
                        int index = i * 3 + j * m_dh->columns * 3 + slize;

                        float x = vecs->at(index);
                        float y = vecs->at(index + 1);
                        float z = vecs->at(index + 2);

                        if (vecs->getUseTex())
                        {
                            r = wxMin(1.0, fabs(x) * bright);
                            g = wxMin(1.0, fabs(y) * bright);
                            b = wxMin(1.0, fabs(z)* bright);
                            a = sqrt(r * r + g * g + b * b);
                            r /= a;
                            g /= a;
                            b /= a;
                        }

                        if (!vecs->getShowFS())
                        {
                            glColor4f(r, g, b, a);
                            glVertex3f((GLfloat) i + .5 + x / 2., (GLfloat) j + .5 + y / 2.,
                                    (GLfloat) m_dh->zSlize + .5 + z / 2.);
                            glVertex3f((GLfloat) i + .5 - x / 2., (GLfloat) j + .5 - y / 2.,
                                    (GLfloat) m_dh->zSlize + .5 - z / 2.);
                        }
                        else
                        {
                            if (vecs->at(index + 2) <= 0.0)
                            {
                                glColor4f(r, g, b, a);
                                if ( topview )
                                {
                                    glVertex3f((GLfloat) i + .5 + x / 2., (GLfloat) j + .5 + y / 2.,
                                            (GLfloat) m_dh->zSlize + .4);
                                    glVertex3f((GLfloat) i + .5, (GLfloat) j + .5, (GLfloat) m_dh->zSlize + .4);
                                    glColor4f(r * dull, g * dull, b * dull, a);
                                    glVertex3f((GLfloat) i + .5 - x / 2., (GLfloat) j + .5 - y / 2.,
                                            (GLfloat) m_dh->zSlize + .4);
                                    glVertex3f((GLfloat) i + .5, (GLfloat) j + .5, (GLfloat) m_dh->zSlize + .4);
                                }
                                else
                                {
                                    glVertex3f((GLfloat) i + .5 - x / 2., (GLfloat) j + .5 - y / 2.,
                                            (GLfloat) m_dh->zSlize + .6);
                                    glVertex3f((GLfloat) i + .5, (GLfloat) j + .5, (GLfloat) m_dh->zSlize + .6);
                                    glColor4f(r * dull, g * dull, b * dull, a);
                                    glVertex3f((GLfloat) i + .5 + x / 2., (GLfloat) j + .5 + y / 2.,
                                            (GLfloat) m_dh->zSlize + .6);
                                    glVertex3f((GLfloat) i + .5, (GLfloat) j + .5, (GLfloat) m_dh->zSlize + .6);
                                }
                            }
                            else
                            {
                                glColor4f(r * dull, g * dull, b * dull, a);
                                if ( topview )
                                {
                                    glVertex3f((GLfloat) i + .5 + x / 2., (GLfloat) j + .5 + y / 2.,
                                            (GLfloat) m_dh->zSlize + .4);
                                    glVertex3f((GLfloat) i + .5, (GLfloat) j + .5, (GLfloat) m_dh->zSlize + .4);
                                    glColor4f(r, g, b, a);
                                    glVertex3f((GLfloat) i + .5 - x / 2., (GLfloat) j + .5 - y / 2.,
                                            (GLfloat) m_dh->zSlize + .4);
                                    glVertex3f((GLfloat) i + .5, (GLfloat) j + .5, (GLfloat) m_dh->zSlize + .4);
                                }
                                else
                                {
                                    glVertex3f((GLfloat) i + .5 - x / 2., (GLfloat) j + .5 - y / 2.,
                                            (GLfloat) m_dh->zSlize + .6);
                                    glVertex3f((GLfloat) i + .5, (GLfloat) j + .5, (GLfloat) m_dh->zSlize + .6);
                                    glColor4f(r, g, b, a);
                                    glVertex3f((GLfloat) i + .5 + x / 2., (GLfloat) j + .5 + y / 2.,
                                            (GLfloat) m_dh->zSlize + .6);
                                    glVertex3f((GLfloat) i + .5, (GLfloat) j + .5, (GLfloat) m_dh->zSlize + .6);
                                }
                            }
                        }
                    }
                }
            }
            if (m_dh->showCoronal)
            {
                for (int i = 0; i < m_dh->columns; ++i)
                {
                    for (int j = 0; j < m_dh->frames; ++j)
                    {
                        int slize = (int)(m_dh->ySlize * m_dh->columns * 3);
                        int index = i * 3 + slize + j * m_dh->columns * m_dh->rows * 3;

                        float x = vecs->at(index);
                        float y = vecs->at(index + 1);
                        float z = vecs->at(index + 2);

                        if (vecs->getUseTex())
                        {
                            r = wxMin(1.0, fabs(x)* bright);
                            g = wxMin(1.0, fabs(y)* bright);
                            b = wxMin(1.0, fabs(z)* bright);
                            a = sqrt(r * r + g * g + b * b);
                            r /= a;
                            g /= a;
                            b /= a;
                        }
                        if (!vecs->getShowFS())
                        {
                            glColor4f(r, g, b, a);
                            glVertex3f((GLfloat) i + .5 + x / 2., (GLfloat) m_dh->ySlize + .5 + y / 2.,
                                    (GLfloat) j + .5 + z / 2.);
                            glVertex3f((GLfloat) i + .5 - x / 2., (GLfloat) m_dh->ySlize + .5 - y / 2.,
                                    (GLfloat) j + .5 - z / 2.);
                        }
                        else
                        {
                            if (vecs->at(index + 1) <= 0.0)
                            {
                                glColor4f(r, g, b, a);
                                if ( frontview )
                                {
                                    glVertex3f((GLfloat) i + .5 + x / 2., (GLfloat) m_dh->ySlize + .4,
                                            (GLfloat) j + .5 + z / 2.);
                                    glVertex3f((GLfloat) i + .5, (GLfloat) m_dh->ySlize + .4, (GLfloat) j + .5);
                                    glColor4f(r * dull, g * dull, b * dull, a);
                                    glVertex3f((GLfloat) i + .5 - x / 2., (GLfloat) m_dh->ySlize + .4,
                                            (GLfloat) j + .5 - z / 2.);
                                    glVertex3f((GLfloat) i + .5, (GLfloat) m_dh->ySlize + .4, (GLfloat) j + .5);
                                }
                                else
                                {
                                    glVertex3f((GLfloat) i + .5 - x / 2., (GLfloat) m_dh->ySlize + .6,
                                            (GLfloat) j + .5 - z / 2.);
                                    glVertex3f((GLfloat) i + .5, (GLfloat) m_dh->ySlize + .6, (GLfloat) j + .5);
                                    glColor4f(r * dull, g * dull, b * dull, a);
                                    glVertex3f((GLfloat) i + .5 + x / 2., (GLfloat) m_dh->ySlize + .6,
                                            (GLfloat) j + .5 + z / 2.);
                                    glVertex3f((GLfloat) i + .5, (GLfloat) m_dh->ySlize + .6, (GLfloat) j + .5);
                                }
                            }

                            else
                            {
                                glColor4f(r * dull, g * dull, b * dull, a);
                                if ( frontview )
                                {
                                    glVertex3f((GLfloat) i + .5 + x / 2., (GLfloat) m_dh->ySlize + .4,
                                            (GLfloat) j + .5 + z / 2.);
                                    glVertex3f((GLfloat) i + .5, (GLfloat) m_dh->ySlize + .4, (GLfloat) j + .5);
                                    glColor4f(r, g, b, a);
                                    glVertex3f((GLfloat) i + .5 - x / 2., (GLfloat) m_dh->ySlize + .4,
                                            (GLfloat) j + .5 - z / 2.);
                                    glVertex3f((GLfloat) i + .5, (GLfloat) m_dh->ySlize + .4, (GLfloat) j + .5);

                                }
                                else
                                {
                                    glVertex3f((GLfloat) i + .5 - x / 2., (GLfloat) m_dh->ySlize + .6,
                                            (GLfloat) j + .5 - z / 2.);
                                    glVertex3f((GLfloat) i + .5, (GLfloat) m_dh->ySlize + .6, (GLfloat) j + .5);
                                    glColor4f(r, g, b, a);
                                    glVertex3f((GLfloat) i + .5 + x / 2., (GLfloat) m_dh->ySlize + .6,
                                            (GLfloat) j + .5 + z / 2.);
                                    glVertex3f((GLfloat) i + .5, (GLfloat) m_dh->ySlize + .6, (GLfloat) j + .5);
                                }
                            }
                        }
                    }
                }
            }
            if (m_dh->showSagittal)
            {
                for (int i = 0; i < m_dh->rows; ++i)
                {
                    for (int j = 0; j < m_dh->frames; ++j)
                    {
                        int slize = (int)(m_dh->xSlize * 3);
                        int index = slize + i * m_dh->columns * 3 + j * m_dh->columns * m_dh->rows * 3;

                        float x = vecs->at(index);
                        float y = vecs->at(index + 1);
                        float z = vecs->at(index + 2);

                        if (vecs->getUseTex())
                        {
                            r = wxMin(1.0, fabs(x)* bright);
                            g = wxMin(1.0, fabs(y)* bright);
                            b = wxMin(1.0, fabs(z)* bright);
                            a = sqrt(r * r + g * g + b * b);
                            r /= a;
                            g /= a;
                            b /= a;
                        }
                        if (!vecs->getShowFS())
                        {
                            glColor4f(r, g, b, a);
                            glVertex3f((GLfloat)m_dh->xSlize + .5 + x / 2., (GLfloat)i + .5 + y / 2.,
                                    (GLfloat) j + .5 + z / 2.);
                            glVertex3f((GLfloat)m_dh->xSlize + .5 - x / 2., (GLfloat)i + .5 - y / 2.,
                                    (GLfloat) j + .5 - z / 2.);
                        }
                        else
                        {
                            if (vecs->at(index) <= 0.0)
                            {
                                glColor4f(r, g, b, a);
                                if ( leftview )
                                {
                                    glVertex3f((GLfloat) m_dh->xSlize + .4, (GLfloat) i + .5 + y / 2.,
                                            (GLfloat) j + .5 + z / 2.);
                                    glVertex3f((GLfloat) m_dh->xSlize + .4, (GLfloat) i + .5, (GLfloat) j + .5);
                                    glColor4f(r * dull, g * dull, b * dull, a);
                                    glVertex3f((GLfloat) m_dh->xSlize + .4, (GLfloat) i + .5 - y / 2.,
                                            (GLfloat) j + .5 - z / 2.);
                                    glVertex3f((GLfloat) m_dh->xSlize + .4, (GLfloat) i + .5, (GLfloat) j + .5);

                                }
                                else
                                {
                                    glVertex3f((GLfloat) m_dh->xSlize + .6, (GLfloat) i + .5 - y / 2.,
                                            (GLfloat) j + .5 - z / 2.);
                                    glVertex3f((GLfloat) m_dh->xSlize + .6, (GLfloat) i + .5, (GLfloat) j + .5);
                                    glColor4f(r * dull, g * dull, b * dull, a);
                                    glVertex3f((GLfloat) m_dh->xSlize + .6, (GLfloat) i + .5 + y / 2.,
                                            (GLfloat) j + .5 + z / 2.);
                                    glVertex3f((GLfloat) m_dh->xSlize + .6, (GLfloat) i + .5, (GLfloat) j + .5);
                                }
                            }
                            else
                            {
                                glColor4f(r * dull, g * dull, b * dull, a);
                                if ( leftview )
                                {
                                    glVertex3f((GLfloat) m_dh->xSlize + .4, (GLfloat) i + .5 + y / 2.,
                                            (GLfloat) j + .5 + z / 2.);
                                    glVertex3f((GLfloat) m_dh->xSlize + .4, (GLfloat) i + .5, (GLfloat) j + .5);
                                    glColor4f(r, g, b, a);
                                    glVertex3f((GLfloat) m_dh->xSlize + .4, (GLfloat) i + .5 - y / 2.,
                                            (GLfloat) j + .5 - z / 2.);
                                    glVertex3f((GLfloat) m_dh->xSlize + .4, (GLfloat) i + .5, (GLfloat) j + .5);

                                }
                                else
                                {
                                    glVertex3f((GLfloat) m_dh->xSlize + .6, (GLfloat) i + .5 - y / 2.,
                                            (GLfloat) j + .5 - z / 2.);
                                    glVertex3f((GLfloat) m_dh->xSlize + .6, (GLfloat) i + .5, (GLfloat) j + .5);
                                    glColor4f(r, g, b, a);
                                    glVertex3f((GLfloat) m_dh->xSlize + .6, (GLfloat) i + .5 + y / 2.,
                                            (GLfloat) j + .5 + z / 2.);
                                    glVertex3f((GLfloat) m_dh->xSlize + .6, (GLfloat) i + .5, (GLfloat) j + .5);
                                }
                            }
                        }
                    }
                }
            }

            for (int j = 0; j < m_dh->mainFrame->m_listCtrl->GetItemCount(); ++j)
            {
                DatasetInfo* mesh = (DatasetInfo*) m_dh->mainFrame->m_listCtrl->GetItemData(j);

                if (mesh->getType() == IsoSurface_ && mesh->getShow())
                {
                    CIsoSurface* surf = (CIsoSurface*) mesh;
                    std::vector<Vector> positions = surf->getSurfaceVoxelPositions();
                    for (size_t k = 0; k < positions.size(); ++k)
                    {
                        int index = (int) positions[k].x * 3 + (int) positions[k].y * m_dh->columns * 3
                                + (int) positions[k].z * m_dh->rows * m_dh->columns * 3;

                        float x = vecs->at(index);
                        float y = vecs->at(index + 1);
                        float z = vecs->at(index + 2);

                        if (vecs->getUseTex())
                        {
                            r = wxMin(1.0, fabs(x)* bright);
                            g = wxMin(1.0, fabs(y)* bright);
                            b = wxMin(1.0, fabs(z)* bright);
                            a = sqrt(r * r + g * g + b * b);
                            r /= a;
                            g /= a;
                            b /= a;
                        }

                        glColor4f(r, g, b, 1.0);
                        glVertex3f(positions[k].x - x / 2., positions[k].y - y / 2., positions[k].z - z / 2.);
                        glVertex3f(positions[k].x + x / 2., positions[k].y + y / 2., positions[k].z + z / 2.);
                    }
                }

                else if (mesh->getType() == Surface_ && mesh->getShow())
                {
                    Surface* surf = (Surface*) mesh;
                    std::vector<Vector> positions = surf->getSurfaceVoxelPositions();

                    for (size_t k = 0; k < positions.size(); ++k)
                    {
                        int index = (int) positions[k].x * 3 + (int) positions[k].y * m_dh->columns * 3
                                + (int) positions[k].z * m_dh->rows * m_dh->columns * 3;

                        float x = vecs->at(index);
                        float y = vecs->at(index + 1);
                        float z = vecs->at(index + 2);

                        if (vecs->getUseTex())
                        {
                            r = wxMin(1.0, fabs(x)* bright);
                            g = wxMin(1.0, fabs(y)* bright);
                            b = wxMin(1.0, fabs(z)* bright);
                            a = sqrt(r * r + g * g + b * b);
                            r /= a;
                            g /= a;
                            b /= a;
                        }

                        glColor4f(r, g, b, 1.0);
                        glVertex3f(positions[k].x - x / 2., positions[k].y - y / 2., positions[k].z - z / 2.);
                        glVertex3f(positions[k].x + x / 2., positions[k].y + y / 2., positions[k].z + z / 2.);
                    }
                }
            }
            glEnd();
        }
    }

    if ( m_dh->GLError() )
        m_dh->printGLError(wxT("draw vectors"));

    glDisable(GL_BLEND);

    glPopAttrib();
}

#ifdef CG_GLYPHS

namespace {
    // is there any better way?
    int upPower2(int i)
    {
        int nx = 1;
        while(nx < i) nx<<=1;
        return nx;
    }

    template<typename T> std::string to_string( const T& i)
    {
        std::ostringstream oss;
        oss << i;
        return oss.str();
    }
}

void TheScene::drawGlyphsOnSurface()
{
    if( (m_dh->xSlize != m_old_xSlice) || (m_dh->ySlize != m_old_ySlice) || (m_dh->zSlize != m_old_zSlice) )
    {//if slice changed
        m_glyphsInitialized = false;
    }
    m_old_xSlice = m_dh->xSlize;
    m_old_ySlice = m_dh->ySlize;
    m_old_zSlice = m_dh->zSlize;
//
//    for (int j = 0; j < m_dh->mainFrame->m_listCtrl->GetItemCount(); ++j)
//    {
//        DatasetInfo* mesh = (DatasetInfo*) m_dh->mainFrame->m_listCtrl->GetItemData(j);
//
//        if( mesh->getType() == IsoSurface_ && mesh->getShow() )
//        {
//            CIsoSurface* surf = (CIsoSurface*) mesh;
//            std::vector<Vector> isoSurfPos = surf->getSurfaceVoxelPositions();
//            if(isoSurfPos.size() != m_old_nbGlyphPositions)
//            m_glyphsInitialized = false;
//            m_old_nbGlyphPositions = isoSurfPos.size();
//        }
//    }

    if( !m_glyphsInitialized )
    {
        m_positions.clear();
        for (int i = 0; i < m_dh->mainFrame->m_listCtrl->GetItemCount(); ++i)
        {
            DatasetInfo* info = (DatasetInfo*) m_dh->mainFrame->m_listCtrl->GetItemData(i);
            if (info->getType() == Tensors_ && info->getShow())
            {
                unsigned int modulo = 4;
                if (m_dh->showAxial)
                {
                    std::vector<Vector> gridPositions;
                    gridPositions.clear();
                    for (int i = 0; i < m_dh->columns; ++i)
                    {
                        for (int j = 0; j < m_dh->rows; ++j)
                        {
                            if( i%modulo == 0 && j%modulo == 0 )
                            {
                                Vector position( i, j, m_dh->zSlize );
                                DatasetInfo* bla = (DatasetInfo*) m_dh->mainFrame->m_listCtrl->GetItemData(0);
                                Anatomy* vals = (Anatomy*)bla;
                                double myval = vals->at(i+ j*m_dh->columns +m_dh->zSlize*m_dh->columns* m_dh->rows );
                                if(myval > bla->getThreshold())
                                m_positions.push_back(position);
                            }
                        }
                    }
                }
                if (m_dh->showCoronal)
                {
                    std::vector<Vector> gridPositions;
                    gridPositions.clear();
                    for (int i = 0; i < m_dh->columns; ++i)
                    {
                        for (int j = 0; j < m_dh->frames; ++j)
                        {
                            if( i%modulo == 0 && j%modulo == 0 )
                            {
                                Vector position( i, m_dh->ySlize, j );
                                DatasetInfo* bla = (DatasetInfo*) m_dh->mainFrame->m_listCtrl->GetItemData(0);
                                Anatomy* vals = (Anatomy*)bla;
                                double myval = vals->at(i+ m_dh->ySlize*m_dh->columns + j*m_dh->columns* m_dh->rows );
                                if(myval > bla->getThreshold())
                                m_positions.push_back(position);
                            }
                        }
                    }
                }
                if (m_dh->showSagittal)
                {
                    std::vector<Vector> gridPositions;
                    gridPositions.clear();
                    for (int i = 0; i < m_dh->rows; ++i)
                    {
                        for (int j = 0; j < m_dh->frames; ++j)
                        {
                            if( i%modulo == 0 && j%modulo == 0 )
                            {
                                Vector position( m_dh->xSlize, i, j );
                                DatasetInfo* bla = (DatasetInfo*) m_dh->mainFrame->m_listCtrl->GetItemData(0);
                                Anatomy* vals = (Anatomy*)bla;
                                double myval = vals->at( m_dh->xSlize+ i*m_dh->columns +j*m_dh->columns* m_dh->rows );
                                if(myval > bla->getThreshold())
                                m_positions.push_back(position);
                            }
                        }
                    }
                }
//                for (int j = 0; j < m_dh->mainFrame->m_listCtrl->GetItemCount(); ++j)
//                {
//                    DatasetInfo* mesh = (DatasetInfo*) m_dh->mainFrame->m_listCtrl->GetItemData(j);
//
//                    if (mesh->getType() == IsoSurface_ && mesh->getShow())
//                    {
//                        CIsoSurface* surf = (CIsoSurface*) mesh;
//                        std::vector<Vector> isoSurfPos = surf->getSurfaceVoxelPositions();
//                        std::vector< Vector > filteredPositions;
//                        filteredPositions.clear();
//                        for (size_t k = 0; k < isoSurfPos.size(); ++k)
//                        {
//                            if( k%100 == 0)
//                            {
//                                m_positions.push_back( isoSurfPos[k] );
//                            }
//
//                        }
//                    }
//
//                    else
//                    if (mesh->getType() == Surface_ && mesh->getShow())
//                    {
//                    }
//                }
            }
        }
    }
    drawGlyphs( m_positions );
}

void TheScene::drawSingleGlyph()
{
    DatasetInfo* info = 0;
    for (int i = 0 ; i < m_dh->mainFrame->m_listCtrl->GetItemCount() ; ++i)
    {
        info = (DatasetInfo*)m_dh->mainFrame->m_listCtrl->GetItemData(i);

        if (info->getType() == Tensors_ && info->getShow())
            break;
    }


    if( (m_dh->xSlize != m_old_xSlice) || (m_dh->ySlize != m_old_ySlice) || (m_dh->zSlize != m_old_zSlice) )
    {//if slice changed


        m_old_xSlice = m_dh->xSlize;
        m_old_ySlice = m_dh->ySlize;
        m_old_zSlice = m_dh->zSlize;

        Vector position( m_dh->xSlize, m_dh->ySlize, m_dh->zSlize );
        m_positions.clear();
        m_positions.push_back(position);
    }

    drawGlyphs( m_positions );
}

void TheScene::drawGlyphs( std::vector< Vector > glyphPositions )
{

    if( glyphPositions.size() == 0 )
    {
        return;
    }

    int order = 0;
    unsigned int components;
    DatasetInfo* info = 0;
    for (int i = 0 ; i < m_dh->mainFrame->m_listCtrl->GetItemCount() ; ++i)
    {
        info = (DatasetInfo*)m_dh->mainFrame->m_listCtrl->GetItemData(i);

        if (info->getType() == Tensors_ && info->getShow())
            break;
    }

    components = info->getBands();

    switch( components )
    {
        case 1:
            order = 0;
            break;
        case 3:
            order = 1;
            break;
        case 6:
            order = 2;
            break;
        case 10:
            order = 3;
            break;
        case 15:
            order = 4;
            break;
        case 21:
            order = 5;
            break;
        case 28:
            order = 6;
            break;
        default:
            std::cout << "Cannot deduce order of tensor from given symmetric components number." << std::endl;
            assert(0);
    }

    if( !m_myGlyphs )
    {
        m_myGlyphs = new FgeImplicitSurfaceGlyphs;
        m_myGlyphs->setScaling(2);
        std::ostringstream oss;
        {
            // TEST POLYNOMIAL:

            const int dim = 3;
            const int ord = order;
            assert((ord & 0x1) == 0);
            unique_count n(dim,ord);
            int nbLoops = n.nbUnique();
            int mult=1.;
            for(int i=1; i<=ord; ++i)
                mult*=i;

            for(int i=0; i< nbLoops; ++i, ++n)
            {
                std::vector<int> hist = n.histogram();
                int m =1;
                for( unsigned int h=0; h< hist.size(); ++h)
                {
                    for(int a=hist[h]; a>0; --a)
                        m *= a;
                }
                if(i != 0) oss << " + ";
                oss << "A" << to_string(i) << "*" << to_string( mult/m ) <<
                (hist[0] == 0 ? "" : "*x^" + to_string(hist[0])) <<
                (hist[1] == 0 ? "" : "*y^" + to_string(hist[1])) <<
                (hist[2] == 0 ? "" : "*z^" + to_string(hist[2]));
            }
            oss << "- 2.*(sqrt(x^2+y^2+z^2))^" << ord+1;
        }
        std::cout << "rendering function: " << oss.str() << std::endl;
        m_myGlyphs->setFunction( oss.str(), 0.f, "", "", "" );
        m_myGlyphs->initiate();
    }
    std::vector< Vector > maskedGlyphPositions;

    if( !m_glyphsInitialized )
    {
        m_glyphsInitialized = true;

        const unsigned int  nbTensors = glyphPositions.size();


        FTensorStorageHelper tsh(3, order);
        std::cout << "getNbSymmetricComponents() : " << tsh.getNbSymmetricComponents() << std::endl;
        int w = upPower2( tsh.getNbSymmetricComponents() );
        int h = upPower2( nbTensors );

        int ww = w;
        if(ww%4 !=0) ww = (w/4) * 4 + 4;
        std::cout<<"WW   "<<ww<<std::endl;
        std::cout<<"H    "<<h<<std::endl;
        std::cout<<"WW*H "<<ww*h<<std::endl;

        GLfloat * data;
        data = new GLfloat[ww*h];

        unsigned int numberOfValues = tsh.getNbSymmetricComponents() * nbTensors;
        std::cout<<"NBTENSORS: " << nbTensors<<std::endl;
        std::cout<<"NBVALUES : " << numberOfValues<<std::endl;


        Anatomy* tensors = (Anatomy*)info;

        for( unsigned int tensId = 0; tensId < nbTensors; ++tensId )
        {
            // std::cout << "COORD " << glyphPositions[tensId][0] << " "<< glyphPositions[tensId][1] << " "<< glyphPositions[tensId][2] << " " << std::endl;
            int x = glyphPositions[tensId][0];
            int y = glyphPositions[tensId][1];
            int z = glyphPositions[tensId][2];
            int index =
                x * components
                + y * m_dh->columns * components
                + z * m_dh->columns * m_dh->rows * components;
//            std::cout << x << " " << y << " " << z << " posId: " << index/components<< " ----- ";

            float maxValueLocal = std::numeric_limits<float>::min();
            float minValueLocal = std::numeric_limits<float>::max();
            for( unsigned int compId = 0; compId < components; ++compId )
            {
                //   std::cout<<"Tensor: " << tensors->at( index + compId ) << std::endl;
                unsigned int dataId = tensId * ww + compId;
                data[dataId]=tensors->at( index + compId );
                maxValueLocal = data[dataId] > maxValueLocal ? data[dataId] : maxValueLocal;
                minValueLocal = data[dataId] < minValueLocal ? data[dataId] : minValueLocal;
//                 std::cout<<"["<<dataId<<","<<data[dataId]<<"] ";
//                std::cout<<data[dataId]<<" ";
            }

            //range of the current tensor's components
            float absMax;
            if( fabs( maxValueLocal )  > fabs( minValueLocal ) )
                absMax = fabs( maxValueLocal );
            else
                absMax = fabs( minValueLocal );

            if( absMax > 1e-9 )
                maskedGlyphPositions.push_back( glyphPositions[tensId] );

        }


        //update data for the maskedGlyphPositions
        const unsigned int nbMaskedTensors = maskedGlyphPositions.size();
        std::cout <<  nbMaskedTensors << " / " << nbTensors << " = " << (float)nbMaskedTensors / (float)nbTensors <<std::endl;
        h = upPower2( nbMaskedTensors );
        std::cout<<"H    "<<h<<std::endl;
        std::cout<<"WW*H "<<ww*h<<std::endl;
        delete data;
        data = new GLfloat[ww*h];

        float maxValue = std::numeric_limits<float>::min();
        float minValue = std::numeric_limits<float>::max();

        //set the data with the maskedGlyphPositions
        for( unsigned int tensId = 0; tensId < nbMaskedTensors; ++tensId )
        {
            // std::cout << "COORD " << glyphPositions[tensId][0] << " "<< glyphPositions[tensId][1] << " "<< glyphPositions[tensId][2] << " " << std::endl;
            int x = maskedGlyphPositions[tensId][0];
            int y = maskedGlyphPositions[tensId][1];
            int z = maskedGlyphPositions[tensId][2];
            int index =
                x * components
                + y * m_dh->columns * components
                + z * m_dh->columns * m_dh->rows * components;
//            std::cout << x << " " << y << " " << z << " posId: " << index/components<< " ----- ";

            float maxValueLocal = std::numeric_limits<float>::min();
            float minValueLocal = std::numeric_limits<float>::max();
            for( unsigned int compId = 0; compId < components; ++compId )
            {
                //   std::cout<<"Tensor: " << tensors->at( index + compId ) << std::endl;
                unsigned int dataId = tensId * ww + compId;
                data[dataId] = tensors->at( index + compId );

                maxValue = data[dataId] > maxValue ? data[dataId] : maxValue;
                minValue = data[dataId] < minValue ? data[dataId] : minValue;

                maxValueLocal = data[dataId] > maxValueLocal ? data[dataId] : maxValueLocal;
                minValueLocal = data[dataId] < minValueLocal ? data[dataId] : minValueLocal;
            }

            //range of the current tensor's components
            float absMax;
            if( fabs( maxValueLocal )  > fabs( minValueLocal ) )
                absMax = fabs( maxValueLocal );
            else
                absMax = fabs( minValueLocal );


            //normalize current tensor using its range and max/min values
            for( unsigned int componentId=0; componentId < components; ++componentId )
            {
                unsigned int dataId = tensId * ww + componentId;
                data[dataId] = ( data[dataId] + absMax ) / (2*absMax);
            }

        }

//
//        std::cout << "################################################# " << std::endl;;
//        std::cout << "# DATA ";
//        for( unsigned int i = 0; i < nbTensors*ww; ++i)
//        {
//            if( i%ww == 0 )
//            {
//		std::cout << std::endl << i << " ";
//            unsigned int glyphId = i/ww;
//            int localindex =
//                glyphPositions[glyphId][0]
//                + glyphPositions[glyphId][1]* m_dh->columns
//                + glyphPositions[glyphId][2] * m_dh->columns * m_dh->rows;
//            std::cout<< glyphPositions[glyphId][0]<<" "<<glyphPositions[glyphId][1]<<" "<<glyphPositions[glyphId][2]<<" ------- " <<localindex<<" ------- ";
//            }
//            std::cout<< (fabs(data[i])>1e-9 ? data[i]:0) << " ";
//        }
//        std::cout << std::endl<<std::endl;


//         float range = maxValue -minValue;
//         assert(range > 0);
//        unsigned int nbFloats = ww*h;
//         for( unsigned int i = 0; i < nbFloats; ++i)
//         {
//             data[i] = ( data[i] - minValue ) / range;
//         }






//
//        double dscale=10;
//
//        for( unsigned int i=0; i< nbFloats; ++i)
//        {
//            data[i] = data[i]*dscale;
//        }

//        std::cout << "################################################# range: " << maxValue<<"-"<<minValue<<"="<<range << std::endl;;
//        std::cout << "# DATA scaled ";
//         unsigned int blaaaaa=0;
//         for( unsigned int i = 0; i < nbFloats; ++i)
//         {
//             if( i%ww == 0 )
//             {
//             std::cout << std::endl;
//             }
//             std::cout<< data[i] << " ";
//             blaaaaa=i;
//         }
//         std::cout << std::endl<<blaaaaa<<std::endl;
        if( m_myGlyphs )
        {
            m_myGlyphs->setData( maskedGlyphPositions.size() , data, ww, h );
            m_myGlyphs->dataToTexture( ww, h );
            for( unsigned int tensId = 0; tensId < maskedGlyphPositions.size(); ++tensId )
            {
                int x = maskedGlyphPositions[tensId][0];
                int y = maskedGlyphPositions[tensId][1];
                int z = maskedGlyphPositions[tensId][2];
                m_myGlyphs->setNewVertex( x, y, z );
            }
        }
        delete[] data;
    }

    if( m_myGlyphs )
    {
        m_myGlyphs->render();
    }

    if ( m_dh->GLError() )
    {
        m_dh->printGLError(wxT("afterender"));
    }
}
#endif //CG_GLYPHS
