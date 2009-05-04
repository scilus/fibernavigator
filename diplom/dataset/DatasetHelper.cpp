/*
 * DatasetHelper.cpp
 *
 *  Created on: 27.07.2008
 *      Author: ralph
 */
#include "DatasetHelper.h"
#include "../gui/mainFrame.h"

#include <memory>

#include "Anatomy.h"
#include "fibers.h"
#include "KdTree.h"
#include "../main.h"
#include "mesh.h"
#include "splinePoint.h"
#include "surface.h"
#include "../gui/myListCtrl.h"

#include "../misc/IsoSurface/CIsoSurface.h"
#include "surface.h"

DatasetHelper::DatasetHelper(MainFrame* mf)
{
    mainFrame = mf;

    rows = 1;
    columns = 1;
    frames = 1;

    xVoxel = 1.0;
    yVoxel = 1.0;
    zVoxel = 1.0;

    countFibers = 0;
    threadsActive = 0;
    anatomy_loaded = false;
    fibers_loaded = false;
    surface_loaded = false;
    vectors_loaded = false;
    tensors_loaded = false;
    surface_isDirty = true;

    scheduledReloadShaders = true;
    scheduledScreenshot = false;
    blendTexOnMesh = true;
    use_lic = false;
    useVBO = true;
    textures = 0;

    colorMap = 0;
    showColorMapLegend = false;
    drawVectors = false;

    showSagittal = true;
    showCoronal = true;
    showAxial = true;
    showCrosshair = false;
    xSlize = 0.5;
    ySlize = 0.5;
    zSlize = 0.5;
    quadrant = 6;

    zoom = 1;
    xMove = 0;
    yMove = 0;

    lastSelectedPoint = NULL;
    lastSelectedBox = NULL;
    scene = NULL;
    anatomyHelper = NULL;
    shaderHelper = NULL;

    boxLockIsOn = false;
    semaphore = false;

    Matrix4fSetIdentity(&m_transform);

    lastError = wxT("");
    lastPath = MyApp::respath + _T("data");
    lastGLError = GL_NO_ERROR;

    lighting = true;
    fibersInverted = false;
    useFakeTubes = false;
    useTransparency = false;
    filterIsoSurf = false;

    m_isDragging = false;
    m_isrDragging = false;
    m_ismDragging = false;

    normalDirection = 1.0;

    showBoxes = true;
    pointMode = false;
    blendAlpha = false;

    m_texAssigned = false;
    m_selBoxChanged = true;

    morphing = false;

    m_scnFileName = _T("");
    m_scenePath = _T("");
    m_screenshotPath = _T("");
    m_scnFileLoaded = false;

    Vector v(0.0, 0.0, 0.0);
    m_freeSlizeOffset = v;
    m_freeSlizeRotation = v;

    geforceLevel = 0;
#ifdef DEBUG
    debugLevel = 0;
#else
    debugLevel = 1;
#endif
}

DatasetHelper::~DatasetHelper()
{

    printDebug(_T("execute dataset helper destructor"),0);
    if (scene)
        delete scene;
    if (anatomyHelper)
        delete anatomyHelper;
    if (lastSelectedPoint)
        delete lastSelectedPoint;
    printDebug(_T("dataset helper destructor done"),0);
}

bool DatasetHelper::load(int index)
{
    wxArrayString fileNames;
    wxString caption= wxT("Choose a file");
    wxString
            wildcard=	wxT("*.*|*.*|Nifti (*.nii)|*.nii*|Mesh files (*.mesh)|*.mesh|"
                    "Mesh files (*.surf)|*.surf|Mesh files (*.dip)|*.dip|"
                    "Fibers VTK (*.fib)|*.fib|Fibers PTK (*.bundlesdata)|*.bundlesdata|"
                    "Scene Files (*.scn)|*.scn");
    wxString defaultDir = wxEmptyString;
    wxString defaultFilename = wxEmptyString;
    wxFileDialog dialog(mainFrame, caption, defaultDir, defaultFilename,
            wildcard, wxOPEN | wxFD_MULTIPLE);
    dialog.SetFilterIndex(index);
    dialog.SetDirectory(lastPath);
    if (dialog.ShowModal() == wxID_OK)
    {
        lastPath = dialog.GetDirectory();
        dialog.GetPaths(fileNames);
    }
    else
        return true;

    bool flag = true;
    for (size_t i = 0; i < fileNames.size() ; ++i)
    {
        if ( !load(fileNames[i]) && flag)
            flag = false;
    }

    return flag;
}

bool DatasetHelper::load(wxString filename, bool createBox, float threshold,
        bool active, bool showFS, bool useTex)
{
    // check if dataset is already loaded and ignore it if yes
    if (fileNameExists(filename))
    {
        lastError = wxT("dataset already loaded");
        return false;
    }

    // check if filename is valid
    if ( !wxFile::Exists(filename) )
    {
        printf("File ");
        printwxT(filename);
        printf(" doesn't exist!\n");
        lastError = wxT("File doesn't exist!");
        return false;
    }

    // check file extension
    wxString ext = filename.AfterLast('.');

    if (ext == wxT("scn"))
    {
        if (!loadScene(filename))
        {
            return false;
        }
        m_selBoxChanged = true;
        mainFrame->refreshAllGLWidgets();

#ifdef __WXMSW__
        m_scnFileName = filename.AfterLast('\\');
        m_scenePath = filename.BeforeLast('\\');
#else
        m_scnFileName = filename.AfterLast('/');
        m_scenePath = filename.BeforeLast('/');
#endif
        m_scnFileLoaded = true;
        return true;
    }

    else if (ext == _T("nii") || ext == _T("gz"))
    {
        Anatomy *anatomy = new Anatomy(this);

        if (anatomy->load(filename))
        {
            anatomy->setThreshold(threshold);
            anatomy->setShow(active);
            anatomy->setShowFS(showFS);
            anatomy->setuseTex(useTex);
            finishLoading(anatomy);
            return true;
        }
        else
        {
            return false;
        }
    }

    else if (ext == _T("mesh") || ext == _T("surf") || ext == _T("dip"))
    {
        if (!anatomy_loaded)
        {
            lastError = wxT("no anatomy file loaded");
            return false;
        }
        Mesh *mesh = new Mesh(this);
        if (mesh->load(filename))
        {
            mesh->setThreshold(threshold);
            mesh->setShow(active);
            mesh->setShowFS(showFS);
            mesh->setuseTex(useTex);
            finishLoading(mesh);
            return true;
        }
        return false;
    }

    else if (ext == _T("fib") || ext == _T("bundlesdata") || ext == _T("Bfloat"))
    {
        if (!anatomy_loaded)
        {
            lastError = wxT("no anatomy file loaded");
            return false;
        }
        if (fibers_loaded)
        {
            lastError = wxT("fibers already loaded");
            return false;
        }
        Fibers *fibers = new Fibers(this);
        if (fibers->load(filename))
        {
            fibers_loaded = true;

            std::vector<std::vector<SelectionBox*> > boxes = getSelectionBoxes();
            for (unsigned int i = 0; i < boxes.size() ; ++i)
            {
                for (unsigned int j = 0; j < boxes[i].size() ; ++j)
                {
                    boxes[i][j]->m_inBox.resize(countFibers, sizeof(bool));
                    for (unsigned int k = 0; k < countFibers; ++k)
                    {
                        boxes[i][j]->m_inBox[k] = 0;
                    }
                    boxes[i][j]->setDirty(true);

                }
            }

            fibers->setThreshold(threshold);
            fibers->setShow(active);
            fibers->setShowFS(showFS);
            fibers->setuseTex(useTex);
            finishLoading(fibers);
            return true;
        }
        return false;
    }

    lastError = wxT("unsupported file format");
    return false;
}

void DatasetHelper::finishLoading(DatasetInfo *info)
{
#ifdef __WXMAC__
    // insert at zero is a well-known bug on OSX, so we append there...
    // http://trac.wxwidgets.org/ticket/4492
    long id = mainFrame->m_listCtrl->GetItemCount();
#else
    long id = 0;
#endif
    mainFrame->m_listCtrl->InsertItem(id, wxT(""), 0);
    if (info->getShow())
        mainFrame->m_listCtrl->SetItem(id, 0, wxT(""), 0);
    else
        mainFrame->m_listCtrl->SetItem(id, 0, wxT(""), 1);

    if (!info->getShowFS())
        mainFrame->m_listCtrl->SetItem(id, 1, info->getName().BeforeFirst('.') + wxT("*"));
    else
        mainFrame->m_listCtrl->SetItem(id, 1, info->getName().BeforeFirst('.'));

    if (!info->getUseTex())
        mainFrame->m_listCtrl->SetItem(0, 2, wxT("(") + wxString::Format(wxT("%.2f"),
                (info->getThreshold()) * info->getOldMax()) + wxT(")"));
    else
        mainFrame->m_listCtrl->SetItem(id, 2, wxString::Format(wxT("%.2f"), info->getThreshold()));

    mainFrame->m_listCtrl->SetItem(id, 3, wxT(""), 1);
    mainFrame->m_listCtrl->SetItemData(id, (long) info);
    mainFrame->m_listCtrl->SetItemState(id, wxLIST_STATE_SELECTED,
            wxLIST_STATE_SELECTED);

    mainFrame->GetStatusBar()->SetStatusText(wxT("Ready"), 1);
    mainFrame->GetStatusBar()->SetStatusText(info->getName() + wxT(" loaded"), 2);

    if (mainFrame->m_listCtrl->GetItemCount() == 1)
    {
        mainFrame->m_xSlider->SetMax(wxMax(2,columns-1));
        mainFrame->m_xSlider->SetValue(columns / 2);
        mainFrame->m_ySlider->SetMax(wxMax(2,rows-1));
        mainFrame->m_ySlider->SetValue(rows / 2);
        mainFrame->m_zSlider->SetMax(wxMax(2,frames-1));
        mainFrame->m_zSlider->SetValue(frames / 2);
        mainFrame->m_tSlider->SetValue(0);
        updateView(mainFrame->m_xSlider->GetValue(),
                mainFrame->m_ySlider->GetValue(),
                mainFrame->m_zSlider->GetValue());

        mainFrame->m_mainGL->changeOrthoSize();
        mainFrame->m_gl0->changeOrthoSize();
        mainFrame->m_gl1->changeOrthoSize();
        mainFrame->m_gl2->changeOrthoSize();
    }

    mainFrame->refreshAllGLWidgets();

}

bool DatasetHelper::fileNameExists(wxString filename)
{
    int countDatasets = mainFrame->m_listCtrl->GetItemCount();
    if (countDatasets == 0)
        return false;

    for (int i = 0; i < countDatasets; ++i)
    {
        DatasetInfo* info =
                (DatasetInfo*) mainFrame->m_listCtrl->GetItemData(i);
        if (info->getPath() == filename)
        {
            return true;
        }
    }
    return false;

}

bool DatasetHelper::loadScene(wxString filename)
{
    /*
     * Variables to store the slice postions in, have to be set after loading
     * the anatomy files
     */
    long xp, yp, zp;
    xp = yp = zp = 0;

    double r00, r10, r20, r01, r11, r21, r02, r12, r22;
    r10 = r20 = r01 = r21 = r02 = r12 = 0;
    r00 = r11 = r22 = 1;

    wxXmlDocument doc;
    if (!doc.Load(filename))
        return false;

    wxXmlNode *child = doc.GetRoot()->GetChildren();
    while (child)
    {
        if (child->GetName() == wxT("anatomy"))
        {
            wxString srows = child->GetPropVal(wxT("rows"), wxT("1"));
            wxString scolumns = child->GetPropVal(wxT("columns"), wxT("1"));
            wxString sframes = child->GetPropVal(wxT("frames"), wxT("1"));
            long _rows, _columns, _frames;
            srows.ToLong(&_rows, 10);
            scolumns.ToLong(&_columns, 10);
            sframes.ToLong(&_frames, 10);
            if (anatomy_loaded)
            {
                if ((_rows != rows) || (_columns != columns) || (_frames
                        != frames))
                {
                    lastError = wxT("dimensions of loaded files must be the same");
                    return false;
                }
            }
            else
            {
                rows = _rows;
                columns = _columns;
                frames = _frames;
                anatomy_loaded = true;
            }
        }

        else if (child->GetName() == wxT("position"))
        {
            wxString xPos = child->GetPropVal(wxT("x"), wxT("1"));
            wxString yPos = child->GetPropVal(wxT("y"), wxT("1"));
            wxString zPos = child->GetPropVal(wxT("z"), wxT("1"));
            xPos.ToLong(&xp, 10);
            yPos.ToLong(&yp, 10);
            zPos.ToLong(&zp, 10);
        }

        else if (child->GetName() == wxT("rotation"))
        {
            wxString rot00 = child->GetPropVal(wxT("rot00"), wxT("1"));
            wxString rot10 = child->GetPropVal(wxT("rot10"), wxT("1"));
            wxString rot20 = child->GetPropVal(wxT("rot20"), wxT("1"));
            wxString rot01 = child->GetPropVal(wxT("rot01"), wxT("1"));
            wxString rot11 = child->GetPropVal(wxT("rot11"), wxT("1"));
            wxString rot21 = child->GetPropVal(wxT("rot21"), wxT("1"));
            wxString rot02 = child->GetPropVal(wxT("rot02"), wxT("1"));
            wxString rot12 = child->GetPropVal(wxT("rot12"), wxT("1"));
            wxString rot22 = child->GetPropVal(wxT("rot22"), wxT("1"));

            rot00.ToDouble(&r00);
            rot10.ToDouble(&r10);
            rot20.ToDouble(&r20);
            rot01.ToDouble(&r01);
            rot11.ToDouble(&r11);
            rot21.ToDouble(&r21);
            rot02.ToDouble(&r02);
            rot12.ToDouble(&r12);
            rot22.ToDouble(&r22);
        }

        else if (child->GetName() == wxT("data"))
        {
            wxXmlNode *datasetnode = child->GetChildren();
            while (datasetnode)
            {
                wxXmlNode *nodes = datasetnode->GetChildren();
                // initialize to mute compiler
                bool active = true;
                bool useTex = true;
                bool showFS = true;
                double threshold = 0.0;
                wxString path;

                while (nodes)
                {
                    if (nodes->GetName() == _T("status"))
                    {
                        active = (nodes->GetPropVal(_T("active"), _T("yes")) == _T("yes") );
                        useTex = (nodes->GetPropVal(_T("useTex"), _T("yes")) == _T("yes") );
                        showFS = (nodes->GetPropVal(_T("showFS"), _T("yes")) == _T("yes") );
                        (nodes->GetPropVal(wxT("threshold"), wxT("0.0"))).ToDouble(&threshold);
                    }
                    else if (nodes->GetName() == _T("path"))
                    {
                        path = nodes->GetNodeContent();
                    }
                    nodes = nodes->GetNext();
                }
                load(path, false, threshold, active, showFS, useTex);
                datasetnode = datasetnode->GetNext();
            }
        }

        else if (child->GetName() == wxT("points"))
        {
            wxXmlNode *pNode = child->GetChildren();
            while (pNode)
            {
                wxString sx = pNode->GetPropVal(wxT("x"), wxT("0.0"));
                wxString sy = pNode->GetPropVal(wxT("y"), wxT("0.0"));
                wxString sz = pNode->GetPropVal(wxT("z"), wxT("0.0"));
                double _x, _y, _z;
                sx.ToDouble(&_x);
                sy.ToDouble(&_y);
                sz.ToDouble(&_z);
                SplinePoint *point = new SplinePoint(_x, _y, _z, this);
                mainFrame->m_treeWidget->AppendItem(mainFrame->m_tPointId, wxT("point"), -1, -1, point);
                pNode = pNode->GetNext();
            }

            if (mainFrame->m_treeWidget->GetChildrenCount(mainFrame->m_tPointId)
                    > 0)
            {
                Surface *surface = new Surface(this);
#ifdef __WXMAC__
                // insert at zero is a well-known bug on OSX, so we append there...
                // http://trac.wxwidgets.org/ticket/4492
                long id = mainFrame->m_listCtrl->GetItemCount();
#else
                long id = 0;
#endif

                mainFrame->m_listCtrl->InsertItem(id, wxT(""), 0);
                mainFrame->m_listCtrl->SetItem(id, 1, _T("spline surface"));
                mainFrame->m_listCtrl->SetItem(id, 2, wxT("0.50"));
                mainFrame->m_listCtrl->SetItem(id, 3, wxT(""), 1);
                mainFrame->m_listCtrl->SetItemData(id, (long)surface);
                mainFrame->m_listCtrl->SetItemState(id, wxLIST_STATE_SELECTED,
                        wxLIST_STATE_SELECTED);
            }
        }

        else if (child->GetName() == wxT("selection_boxes") /*&& TheDataset::fibers_loaded*/)
        {
            wxXmlNode *boxNode = child->GetChildren();
            wxTreeItemId currentMasterId;

            wxString _name, _type, _active, _visible;
            double cx, cy, cz, ix, iy, iz;
            double _cx, _cy, _cz, _ix, _iy, _iz;
            cx = cy = cz = ix = iy = iz = 0;
            _cx = _cy = _cz = _ix = _iy = _iz = 0;

            while (boxNode)
            {
                wxXmlNode *infoNode = boxNode->GetChildren();
                while (infoNode)
                {

                    if (infoNode->GetName() == wxT("status"))
                    {
                        _type = infoNode->GetPropVal(wxT("type"), wxT("MASTER"));
                        _active = infoNode->GetPropVal(wxT("active"), wxT("yes"));
                        _visible = infoNode->GetPropVal(wxT("visible"), wxT("yes"));

                    }

                    if (infoNode->GetName() == wxT("name"))
                    {
                        _name = infoNode->GetPropVal(wxT("string"), wxT("box"));

                    }

                    if (infoNode->GetName() == wxT("size"))
                    {
                        wxString sx = infoNode->GetPropVal(wxT("x"), wxT("0.0"));
                        wxString sy = infoNode->GetPropVal(wxT("y"), wxT("0.0"));
                        wxString sz = infoNode->GetPropVal(wxT("z"), wxT("0.0"));
                        sx.ToDouble(&_ix);
                        sy.ToDouble(&_iy);
                        sz.ToDouble(&_iz);

                    }

                    if (infoNode->GetName() == wxT("center"))
                    {
                        wxString sx = infoNode->GetPropVal(wxT("x"), wxT("0.0"));
                        wxString sy = infoNode->GetPropVal(wxT("y"), wxT("0.0"));
                        wxString sz = infoNode->GetPropVal(wxT("z"), wxT("0.0"));
                        sx.ToDouble(&_cx);
                        sy.ToDouble(&_cy);
                        sz.ToDouble(&_cz);

                    }
                    infoNode = infoNode->GetNext();
                }
                Vector vc(_cx, _cy, _cz);
                Vector vs(_ix, _iy, _iz);
                SelectionBox *selBox = new SelectionBox(vc, vs, this);
                selBox->setName(_name);
                selBox->setActive(_active == _T("yes")) ;
                selBox->setVisible(_visible == _T("yes")) ;

                if (_type == wxT("MASTER"))
                {
                    selBox->setIsMaster(true);
                    currentMasterId = mainFrame->m_treeWidget->AppendItem(
                            mainFrame->m_tSelBoxId, selBox->getName(), 0, -1,
                            selBox);
                    mainFrame->m_treeWidget->EnsureVisible(currentMasterId);
                    mainFrame->m_treeWidget->SetItemImage(currentMasterId,
                            selBox->getIcon());
                    mainFrame->m_treeWidget->SetItemBackgroundColour(
                            currentMasterId, *wxCYAN);
                    selBox->setTreeId(currentMasterId);

                }
                else
                {
                    selBox->setNOT(_type == _T("NOT")) ;
                    wxTreeItemId boxId = mainFrame->m_treeWidget->AppendItem(
                            currentMasterId, selBox->getName(), 0, -1, selBox);
                    mainFrame->m_treeWidget->EnsureVisible(boxId);
                    mainFrame->m_treeWidget->SetItemImage(boxId,
                            selBox->getIcon());
                    if (selBox->getNOT())
                        mainFrame->m_treeWidget->SetItemBackgroundColour(boxId,
                                *wxRED);
                    else
                        mainFrame->m_treeWidget->SetItemBackgroundColour(boxId,
                                *wxGREEN);
                    selBox->setTreeId(boxId);
                }
                boxNode = boxNode->GetNext();
            }
        }
        child = child->GetNext();
    }

    mainFrame->m_xSlider->SetValue(xp);
    mainFrame->m_ySlider->SetValue(yp);
    mainFrame->m_zSlider->SetValue(zp);
    updateView(xp, yp, zp);

    m_transform.s.M00 = r00;
    m_transform.s.M10 = r10;
    m_transform.s.M20 = r20;
    m_transform.s.M01 = r01;
    m_transform.s.M11 = r11;
    m_transform.s.M21 = r21;
    m_transform.s.M02 = r02;
    m_transform.s.M12 = r12;
    m_transform.s.M22 = r22;
    mainFrame->m_mainGL->setRotation();

    return true;
}

void DatasetHelper::save(wxString filename)
{
    wxXmlNode *root = new wxXmlNode(NULL, wxXML_ELEMENT_NODE, wxT("scene"));
    wxXmlNode *nodeboxes = new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("selection_boxes"));
    wxXmlNode *nodepoints = new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("points"));
    wxXmlNode *data = new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("data"));
    wxXmlNode *anatomy = new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("anatomy"));
    wxXmlNode *rotation = new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("rotation"));
    wxXmlNode *anatomyPos = new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("position"));

    wxXmlProperty *prop1 = new wxXmlProperty(wxT("rows"), wxString::Format(wxT("%d"), rows));
    wxXmlProperty *prop2 = new wxXmlProperty(wxT("columns"), wxString::Format(wxT("%d"), columns), prop1);
    wxXmlProperty *prop3 = new wxXmlProperty(wxT("frames"), wxString::Format(wxT("%d"), frames), prop2);
    anatomy->AddProperty(prop3);

    wxXmlProperty *rot00 = new wxXmlProperty(wxT("rot00"), wxString::Format(wxT("%.8f"), m_transform.s.M00));
    wxXmlProperty *rot10 = new wxXmlProperty(wxT("rot10"), wxString::Format(wxT("%.8f"), m_transform.s.M10), rot00);
    wxXmlProperty *rot20 = new wxXmlProperty(wxT("rot20"), wxString::Format(wxT("%.8f"), m_transform.s.M20), rot10);
    wxXmlProperty *rot01 = new wxXmlProperty(wxT("rot01"), wxString::Format(wxT("%.8f"), m_transform.s.M01), rot20);
    wxXmlProperty *rot11 = new wxXmlProperty(wxT("rot11"), wxString::Format(wxT("%.8f"), m_transform.s.M11), rot01);
    wxXmlProperty *rot21 = new wxXmlProperty(wxT("rot21"), wxString::Format(wxT("%.8f"), m_transform.s.M21), rot11);
    wxXmlProperty *rot02 = new wxXmlProperty(wxT("rot02"), wxString::Format(wxT("%.8f"), m_transform.s.M02), rot21);
    wxXmlProperty *rot12 = new wxXmlProperty(wxT("rot12"), wxString::Format(wxT("%.8f"), m_transform.s.M12), rot02);
    wxXmlProperty *rot22 = new wxXmlProperty(wxT("rot22"), wxString::Format(wxT("%.8f"), m_transform.s.M22), rot12);
    rotation->AddProperty(rot22);

    int countPoints = mainFrame->m_treeWidget->GetChildrenCount(
            mainFrame->m_tPointId, true);
    wxTreeItemId id, childid;
    wxTreeItemIdValue cookie = 0;
    for (int i = 0; i < countPoints; ++i)
    {
        id = mainFrame->m_treeWidget->GetNextChild(mainFrame->m_tPointId,
                cookie);
        SplinePoint *point =
                (SplinePoint*) (mainFrame->m_treeWidget->GetItemData(id));
        wxXmlNode *pointnode = new wxXmlNode(nodepoints, wxXML_ELEMENT_NODE, wxT("point"));

        wxXmlProperty *propz = new wxXmlProperty(wxT("z"), wxString::Format(wxT("%f"), point->getCenter().z));
        wxXmlProperty *propy = new wxXmlProperty(wxT("y"), wxString::Format(wxT("%f"), point->getCenter().y), propz);
        wxXmlProperty *propx = new wxXmlProperty(wxT("x"), wxString::Format(wxT("%f"), point->getCenter().x), propy);
        pointnode->AddProperty(propx);
    }

    SelectionBox* currentBox;
    std::vector<std::vector<SelectionBox*> > boxes = getSelectionBoxes();
    for (unsigned int i = boxes.size(); i > 0; --i)
    {
        for (unsigned int j = boxes[i - 1].size(); j > 0; --j)
        {
            wxXmlNode *box = new wxXmlNode(nodeboxes, wxXML_ELEMENT_NODE, wxT("box"));
            currentBox = boxes[i - 1][j - 1];
            if ( !currentBox->getIsBox() )
                continue;

            wxXmlNode *center = new wxXmlNode(box, wxXML_ELEMENT_NODE, wxT("center"));
            wxXmlProperty *propz = new wxXmlProperty(wxT("z"), wxString::Format(wxT("%f"), currentBox->getCenter().z));
            wxXmlProperty *propy = new wxXmlProperty(wxT("y"), wxString::Format(wxT("%f"), currentBox->getCenter().y), propz);
            wxXmlProperty *propx = new wxXmlProperty(wxT("x"), wxString::Format(wxT("%f"), currentBox->getCenter().x), propy);
            center->AddProperty(propx);

            wxXmlNode *size = new wxXmlNode(box, wxXML_ELEMENT_NODE, wxT("size"));
            propz = new wxXmlProperty(wxT("z"), wxString::Format(wxT("%f"), currentBox->getSize().z));
            propy = new wxXmlProperty(wxT("y"), wxString::Format(wxT("%f"), currentBox->getSize().y), propz);
            propx = new wxXmlProperty(wxT("x"), wxString::Format(wxT("%f"), currentBox->getSize().x), propy);
            size->AddProperty(propx);

            wxXmlNode *name = new wxXmlNode(box, wxXML_ELEMENT_NODE, wxT("name"));
            wxXmlProperty *propname = new wxXmlProperty(wxT("string"), currentBox->getName());
            name->AddProperty(propname);

            wxXmlNode *status = new wxXmlNode(box, wxXML_ELEMENT_NODE, wxT("status"));
            wxXmlProperty *proptype;
            if (j - 1 == 0)
                proptype = new wxXmlProperty(wxT("type"), wxT("MASTER"));
            else
                proptype = new wxXmlProperty(wxT("type"), currentBox->getNOT() ? wxT("NOT") : wxT("AND"));
            wxXmlProperty *propactive = new wxXmlProperty(wxT("active"), currentBox->getActive() ? wxT("yes") : wxT("no"), proptype);
            wxXmlProperty *propvisible = new wxXmlProperty(wxT("visible"), currentBox->getVisible() ? wxT("yes") : wxT("no"), propactive);
            wxXmlProperty *propisbox = new wxXmlProperty(wxT("isBox"), currentBox->getIsBox() ? wxT("yes") : wxT("no"), propvisible);
            status->AddProperty(propisbox);
        }
    }

    int countTextures = mainFrame->m_listCtrl->GetItemCount();
    if (countTextures == 0)
        return;

    for (int i = 0; i < countTextures; ++i)
    {
        DatasetInfo* info =
                (DatasetInfo*) mainFrame->m_listCtrl->GetItemData(i);
        if (info->getType() < Surface_)
        {
            wxXmlNode *datasetnode = new wxXmlNode(data, wxXML_ELEMENT_NODE, wxT("dataset"));

            wxXmlNode *pathnode = new wxXmlNode(datasetnode, wxXML_ELEMENT_NODE, wxT("path"));
            new wxXmlNode(pathnode, wxXML_TEXT_NODE, wxT("path"), info->getPath());

            wxXmlNode *statusNode = new wxXmlNode(datasetnode, wxXML_ELEMENT_NODE, wxT("status"));
            wxXmlProperty *propT = new wxXmlProperty(wxT("threshold"), wxString::Format(wxT("%.2f"), info->getThreshold()));
            wxXmlProperty *propA = new wxXmlProperty(wxT("active"), info->getShow() ? _T("yes") : _T("no"), propT);
            wxXmlProperty *propF = new wxXmlProperty(wxT("showFS"), info->getShowFS() ? _T("yes") : _T("no"), propA);
            wxXmlProperty *propU = new wxXmlProperty(wxT("useTex"), info->getUseTex() ? _T("yes") : _T("no"), propF);
            statusNode->AddProperty(propU);
        }
    }

    wxXmlProperty *propPosX = new wxXmlProperty(wxT("x"), wxString::Format(wxT("%d"), mainFrame->m_xSlider->GetValue()));
    wxXmlProperty *propPosY = new wxXmlProperty(wxT("y"), wxString::Format(wxT("%d"), mainFrame->m_ySlider->GetValue()), propPosX);
    wxXmlProperty *propPosZ = new wxXmlProperty(wxT("z"), wxString::Format(wxT("%d"), mainFrame->m_zSlider->GetValue()), propPosY);
    anatomyPos->AddProperty(propPosZ);

    wxXmlDocument doc;
    doc.SetRoot(root);
    if (filename.AfterLast('.') != _T("scn"))
        filename += _T(".scn");
    doc.Save(filename, 2);
}

std::vector<std::vector<SelectionBox*> > DatasetHelper::getSelectionBoxes()
{
    std::vector<std::vector<SelectionBox*> > boxes;

    wxTreeItemId id, childid;
    wxTreeItemIdValue cookie = 0;

    id = mainFrame->m_treeWidget->GetFirstChild(mainFrame->m_tSelBoxId, cookie);

    while (id.IsOk() )
    {
        std::vector<SelectionBox*> b;
        b.push_back( (SelectionBox*)(mainFrame->m_treeWidget->GetItemData(id)) );
        wxTreeItemIdValue childcookie = 0;
        childid = mainFrame->m_treeWidget->GetFirstChild(id, childcookie);
        while (childid.IsOk() )
        {
            b.push_back((SelectionBox*)(mainFrame->m_treeWidget->GetItemData(childid)));
            childid = mainFrame->m_treeWidget->GetNextChild(id, childcookie);
        }
        id = mainFrame->m_treeWidget->GetNextChild(mainFrame->m_tSelBoxId,
                cookie);
        boxes.push_back(b);

    }

    return boxes;
}

void DatasetHelper::treeFinished()
{
    threadsActive--;
    if (threadsActive > 0)
        return;
    printDebug(_T("tree finished"),1);
    fibers_loaded = true;
    updateAllSelectionBoxes();
    m_selBoxChanged = true;
    mainFrame->refreshAllGLWidgets();
}

void DatasetHelper::updateAllSelectionBoxes()
{
    std::vector<std::vector<SelectionBox*> > boxes = getSelectionBoxes();
    for (unsigned int i = 0; i < boxes.size(); ++i)
        for (unsigned int j = 0; j < boxes[i].size(); ++j)
            boxes[i][j]->setDirty(true);
}

Vector DatasetHelper::mapMouse2World(int x, int y)
{
    glPushMatrix();
    doMatrixManipulation();

    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLfloat winX, winY;

    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);

    winX = (float) x;
    winY = (float) viewport[3] - (float) y;
    GLdouble posX, posY, posZ;
    gluUnProject(winX, winY, 0, modelview, projection, viewport, &posX, &posY,
            &posZ);
    glPopMatrix();
    Vector v(posX, posY, posZ);
    return v;
}

Vector DatasetHelper::mapMouse2WorldBack(int x, int y)
{
    glPushMatrix();
    doMatrixManipulation();

    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLfloat winX, winY;

    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);

    winX = (float) x;
    winY = (float) viewport[3] - (float) y;
    GLdouble posX, posY, posZ;
    gluUnProject(winX, winY, 1, modelview, projection, viewport, &posX, &posY,
            &posZ);
    glPopMatrix();
    Vector v(posX, posY, posZ);
    return v;
}

bool DatasetHelper::GLError()
{
    lastGLError = glGetError();
    if (lastGLError == GL_NO_ERROR)
        return false;
    return true;
}

bool DatasetHelper::loadTextFile(wxString* string, wxString filename)
{
    wxTextFile file;
    *string = wxT("");
    if (file.Open(filename))
    {
        size_t i;
        for (i = 0; i < file.GetLineCount(); ++i)
        {
            *string += file.GetLine(i);
        }
        return true;
    }
    return false;
}

void DatasetHelper::createIsoSurface()
{
    // check anatomy - quit if not present
    if (!anatomy_loaded)
        return;
    // get selected anatomy dataset
    long item = mainFrame->m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL,
            wxLIST_STATE_SELECTED);
    if (item == -1)
        return;

    DatasetInfo *info = (DatasetInfo*) mainFrame->m_listCtrl->GetItemData(item);
    if (info->getType() > Overlay)
        return;

    Anatomy* anatomy = (Anatomy*) info;

    printDebug(_T("start generating iso surface..."),1);

    CIsoSurface *isosurf = new CIsoSurface(this, anatomy); //->getFloatDataset());
    isosurf->GenerateSurface(0.4f);

    printDebug(_T("iso surface done"),1);

    wxString anatomyName = anatomy->getName().BeforeFirst('.');

    if (isosurf->IsSurfaceValid())
    {
        isosurf->setName(anatomyName + wxT(" (iso surface)"));

#ifdef __WXMAC__
        // insert at zero is a well-known bug on OSX, so we append there...
        // http://trac.wxwidgets.org/ticket/4492
        long id = mainFrame->m_listCtrl->GetItemCount();
#else
        long id = 0;
#endif
        mainFrame->m_listCtrl->InsertItem(id, wxT(""), 0);
        mainFrame->m_listCtrl->SetItem(id, 1, isosurf->getName());
        mainFrame->m_listCtrl->SetItem(id, 2, wxT("0.40"));
        mainFrame->m_listCtrl->SetItem(id, 3, wxT(""), 1);
        mainFrame->m_listCtrl->SetItemData(id, (long) isosurf);
        mainFrame->m_listCtrl->SetItemState(id, wxLIST_STATE_SELECTED,
        wxLIST_STATE_SELECTED);

    }
    else
    {
        printDebug(_T("***ERROR*** surface is not valid"),2);
    }
    mainFrame->refreshAllGLWidgets();

}

void DatasetHelper::createDistanceMap()
{
    // check anatomy - quit if not present
    if (!anatomy_loaded)
        return;
    // get selected anatomy dataset
    long item = mainFrame->m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL,
            wxLIST_STATE_SELECTED);
    if (item == -1)
        return;

    DatasetInfo *info = (DatasetInfo*) mainFrame->m_listCtrl->GetItemData(item);
    if (info->getType() > Overlay)
        return;

    Anatomy* anatomy = (Anatomy*) info;

    printDebug(_T("start generating distance map..."),1);

    Anatomy* newAnatomy = new Anatomy(this, anatomy->getFloatDataset());

    printDebug(_T("distance map done"),1);

    printDebug(_T("start generating iso surface..."),1);

    CIsoSurface *isosurf = new CIsoSurface(this, newAnatomy);
    isosurf->GenerateSurface(0.1f);

    printDebug(_T("iso surface done"),1);

    wxString anatomyName = anatomy->getName().BeforeFirst('.');

    if (isosurf->IsSurfaceValid())
    {
        isosurf->setName(anatomyName + wxT(" (offset)"));

        mainFrame->m_listCtrl->InsertItem(0, wxT(""), 0);
        mainFrame->m_listCtrl->SetItem(0, 1, isosurf->getName());
        mainFrame->m_listCtrl->SetItem(0, 2, wxT("0.10"));
        mainFrame->m_listCtrl->SetItem(0, 3, wxT(""), 1);
        mainFrame->m_listCtrl->SetItemData(0, (long) isosurf);
        mainFrame->m_listCtrl->SetItemState(0, wxLIST_STATE_SELECTED,
        wxLIST_STATE_SELECTED);

    }
    else
    {
        printDebug(_T("***ERROR*** surface is not valid"),2);
    }

    delete newAnatomy;
    mainFrame->refreshAllGLWidgets();
}

void DatasetHelper::createCutDataset()
{
    // check anatomy - quit if not present
    if (!anatomy_loaded)
        return;
    // get selected anatomy dataset
    long item = mainFrame->m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL,
            wxLIST_STATE_SELECTED);
    if (item == -1)
        return;

    DatasetInfo *info = (DatasetInfo*) mainFrame->m_listCtrl->GetItemData(item);
    if (info->getType() > Overlay)
        return;

    Anatomy* anatomy = (Anatomy*) info;
    Anatomy* newAnatomy = new Anatomy(this);
    newAnatomy->setZero(columns, rows, frames);

    std::vector<std::vector<SelectionBox*> > boxes = getSelectionBoxes();
    int x1, x2, y1, y2, z1, z2;

    for (unsigned int i = 0; i < boxes.size() ; ++i)
    {
        for (unsigned int j = 0; j < boxes[i].size() ; ++j)
        {
            if (boxes[i][j]->getShow() )
            {
                x1 = (int) (boxes[i][j]->getCenter().x - boxes[i][j]->getSize().x /2);
                x2 = (int) (boxes[i][j]->getCenter().x + boxes[i][j]->getSize().x /2);
                y1 = (int) (boxes[i][j]->getCenter().y - boxes[i][j]->getSize().y /2);
                y2 = (int) (boxes[i][j]->getCenter().y + boxes[i][j]->getSize().y /2);
                z1 = (int) (boxes[i][j]->getCenter().z - boxes[i][j]->getSize().z /2);
                z2 = (int) (boxes[i][j]->getCenter().z + boxes[i][j]->getSize().z /2);

                x1 = wxMax(0, wxMin(x1, columns));
                x2 = wxMax(0, wxMin(x2, columns));
                y1 = wxMax(0, wxMin(y1, rows));
                y2 = wxMax(0, wxMin(y2, rows));
                z1 = wxMax(0, wxMin(z1, frames));
                z2 = wxMax(0, wxMin(z2, frames));

                float* src = anatomy->getFloatDataset();
                float* dst = newAnatomy->getFloatDataset();

                //printf ("x1: %d   x2: %d   y1: %d   y2: %d   z1: %d   z2: %d\n", x1,x2,y1,y2,z1,z2);

                for (int b=z1; b < z2; ++b)
                {
                    for (int r=y1; r < y2; ++r)
                    {
                        for (int c=x1; c<x2; ++c)
                        {
                            dst[b*rows*columns + r*columns + c]
                                    = src[b*rows*columns + r*columns + c];
                        }
                    }
                }

            }
        }
    }

    newAnatomy->setName(anatomy->getName().BeforeFirst('.') + wxT(" (cut)"));
    newAnatomy->setType(anatomy->getType());

    mainFrame->m_listCtrl->InsertItem(0, wxT(""), 0);
    mainFrame->m_listCtrl->SetItem(0, 1, newAnatomy->getName());
    mainFrame->m_listCtrl->SetItem(0, 2, wxT("0.00"));
    mainFrame->m_listCtrl->SetItem(0, 3, wxT(""), 1);
    mainFrame->m_listCtrl->SetItemData(0, (long) newAnatomy);
    mainFrame->m_listCtrl->SetItemState(0, wxLIST_STATE_SELECTED,
    wxLIST_STATE_SELECTED);

    mainFrame->refreshAllGLWidgets();
}

void DatasetHelper::changeZoom(int z)
{
    float delta = ((int)zoom)*0.1f;
    z >= 0 ? zoom = wxMin(10, zoom+delta) : zoom = wxMax(1, zoom-delta);
}

void DatasetHelper::moveScene(int x, int y)
{
    xMove -= x;
    yMove += y;
}

void DatasetHelper::doMatrixManipulation()
{
    float max = (float)wxMax(columns * xVoxel, wxMax(rows * yVoxel, frames * zVoxel))/2.0;
    glTranslatef(max + xMove, max + yMove, max);
    glScalef(zoom, zoom, zoom);
    glMultMatrixf(m_transform.M);
    glTranslatef(-columns * xVoxel/2.0, -rows * yVoxel/2.0, -frames * zVoxel/2.0);
}

void DatasetHelper::updateView(float x, float y, float z)
{
    xSlize = x;
    ySlize = y;
    zSlize = z;

    if (boxLockIsOn && !semaphore)
    {
        boxAtCrosshair->setCenter(x, y, z);
    }
}

bool DatasetHelper::getFiberDataset(Fibers *&f)
{
    f = NULL;
    for (int i = 0; i < mainFrame->m_listCtrl->GetItemCount() ; ++i)
    {
        DatasetInfo* info =
                (DatasetInfo*) mainFrame->m_listCtrl->GetItemData(i);
        if (info->getType() == Fibers_)
        {
            f = (Fibers*) mainFrame->m_listCtrl->GetItemData(i);
            return true;
        }
    }
    return false;
}

bool DatasetHelper::getSurfaceDataset(Surface *&s)
{
    s = NULL;
    for (int i = 0; i < mainFrame->m_listCtrl->GetItemCount() ; ++i)
    {
        DatasetInfo* info =
                (DatasetInfo*) mainFrame->m_listCtrl->GetItemData(i);
        if (info->getType() == Surface_)
        {
            s = (Surface*) mainFrame->m_listCtrl->GetItemData(i);
            return true;
        }
    }
    return false;
}

float* DatasetHelper::getVectorDataset()
{
    if (!vectors_loaded)
        return NULL;
    for (int i = 0; i < mainFrame->m_listCtrl->GetItemCount() ; ++i)
    {
        DatasetInfo* info =
                (DatasetInfo*) mainFrame->m_listCtrl->GetItemData(i);
        if (info->getType() == Vectors_)
        {
            Anatomy* a = (Anatomy*) mainFrame->m_listCtrl->GetItemData(i);
            return a->getFloatDataset();
        }
    }
    vectors_loaded = false;
    return NULL;
}

TensorField* DatasetHelper::getTensorField()
{
    if (!tensors_loaded)
        return NULL;
    for (int i = 0; i < mainFrame->m_listCtrl->GetItemCount() ; ++i)
    {
        DatasetInfo* info =
                (DatasetInfo*) mainFrame->m_listCtrl->GetItemData(i);
        if (info->getType() == Tensors_ || info->getType() == Vectors_)
        {
            Anatomy* a = (Anatomy*) mainFrame->m_listCtrl->GetItemData(i);
            return a->getTensorField();
        }
    }
    tensors_loaded = false;
    return NULL;
}

void DatasetHelper::printGLError(wxString function)
{
    printDebug(_T("***ERROR***: ") + function, 2);
    printf(" : ERROR: %s\n", gluErrorString(lastGLError));
    //printDebug(wxString::Format(_T("***ERROR***: %s\n"), //gluErrorString(lastGLError)), 2);
}

void DatasetHelper::printTime()
{
    wxDateTime dt = wxDateTime::Now();
    printf("[%02d:%02d:%02d] ", dt.GetHour(), dt.GetMinute(), dt.GetSecond());
}

void DatasetHelper::printwxT(wxString string)
{
    char* cstring;
    cstring = (char*) malloc(string.length()+1);
    strcpy(cstring, (const char*) string.mb_str(wxConvUTF8));
    printf("%s", cstring);
}

void DatasetHelper::printDebug(wxString string, int level)
{
    //
    if (debugLevel > level)
        return;
    printTime();
    printwxT(string + _T("\n"));
}

void DatasetHelper::updateLoadStatus()
{
    bool anatomy_loaded = false;
    bool fibers_loaded = false;
    bool vectors_loaded = false;
    bool tensors_loaded = false;
    bool surface_loaded = false;

    for (int i = 0; i < mainFrame->m_listCtrl->GetItemCount() ; ++i)
    {
        DatasetInfo* info =
                (DatasetInfo*) mainFrame->m_listCtrl->GetItemData(i);
        switch (info->getType())
        {
            case Head_byte:
            case Head_short:
            case Overlay:
            case RGB:
                anatomy_loaded = true;
                break;
            case Vectors_:
                anatomy_loaded = true;
                vectors_loaded = true;
                break;
            case Mesh_:
                break;
            case Tensors_:
                tensors_loaded = true;
                break;
            case Fibers_:
                fibers_loaded = true;
                break;
            case Surface_:
                surface_loaded = true;
                break;
            case IsoSurface_:
                break;
            default:
                break;
        }
    }
}
