#include "curves.h"
#include "theDataset.h"


Curves::Curves()
{
	m_type = not_initialized;
	m_length = 0;
	m_bands = 0;
	m_frames = 0;
	m_rows = 0;
	m_columns = 0;
	m_repn = wxT("");
	m_xVoxel = 0.0;
	m_yVoxel = 0.0;
	m_zVoxel = 0.0;
	is_loaded = false;
	m_highest_value = 1.0;
	m_threshold = 0.10;
	m_show = true;
	m_showFS = true;
	m_useTex = true;
	m_bufferObjects = new GLuint[3];
}

Curves::~Curves()
{
	delete[] m_linePointers;
	delete[] m_pointArray;
	delete[] m_lineArray;
	delete[] m_reverse;
	delete m_kdTree;
	glDeleteBuffers(3, m_bufferObjects);
	TheDataset::fibers_loaded = false;
}

bool Curves::load(wxString filename)
{
	TheDataset::printTime();
	printf("start loading vtk file\n");
	wxFile dataFile;
	wxFileOffset nSize = 0;

	if (dataFile.Open(filename))
	{
		nSize = dataFile.Length();
		if (nSize == wxInvalidOffset) return false;
	}

	wxUint8* buffer = new wxUint8[255];
	dataFile.Read(buffer, (size_t) 255);


	char* temp = new char[256];
	int i = 0;
	int j = 0;
	while (buffer[i] != '\n') {
		++i;
	}
	++i;
	while (buffer[i] != '\n') {
		++i;
	}
	++i;
	while (buffer[i] != '\n') {
		temp[j] = buffer[i];
		++i;
		++j;
	}
	++i;
	temp[j] = 0;
	wxString type(temp, wxConvUTF8);
	if (type == wxT("ASCII")) {
		//ASCII file, maybe later
		return NULL;
	}

	if (type != wxT("BINARY")) {
		//somethingn else, don't know what to do
		return NULL;
	}

	j = 0;
	while (buffer[i] != '\n') {
		++i;
	}
	++i;
	while (buffer[i] != '\n') {
		temp[j] = buffer[i];
		++i;
		++j;
	}
	++i;
	temp[j] = 0;
	wxString points(temp, wxConvUTF8);
	points = points.AfterFirst(' ');
	points = points.BeforeFirst(' ');
	long tempValue;
	if(!points.ToLong(&tempValue, 10)) return false; //can't read point count
	int countPoints = (int)tempValue;

	// start position of the point array in the file
	int pc = i;

	i += (12 * countPoints) +1;
	j = 0;
	dataFile.Seek(i);
	dataFile.Read(buffer, (size_t) 255);
	while (buffer[j] != '\n') {
		temp[j] = buffer[j];
		++i;
		++j;
	}
	++i;
	temp[j] = 0;

	wxString sLines(temp, wxConvUTF8);
	wxString sLengthLines = sLines.AfterLast(' ');
	if(!sLengthLines.ToLong(&tempValue, 10)) return false; //can't read size of lines array
	int lengthLines = (int(tempValue));
	sLines = sLines.AfterFirst(' ');
	sLines = sLines.BeforeFirst(' ');
	if(!sLines.ToLong(&tempValue, 10)) return false; //can't read lines
	int countLines = (int)tempValue;
	// start postion of the line array in the file
	int lc = i;

	i += (lengthLines*4) +1;
	dataFile.Seek(i);
	dataFile.Read(buffer, (size_t) 255);
	j = 0;
	int k = 0;
	// TODO test if there's really a color array;
	while (buffer[k] != '\n') {
		++i;
		++k;
	}
	++k;
	++i;
	while (buffer[k] != '\n') {
		temp[j] = buffer[k];
		++i;
		++j;
		++k;
	}
	++i;
	temp[j] = 0;

	//int cc = i;

	m_lineCount = countLines;
	TheDataset::countFibers = m_lineCount;
	m_pointCount = countPoints;
	m_linePointers = new int[countLines+1];
	m_linePointers[countLines] = countPoints;
	m_reverse = new int[countPoints];
	m_inBox.resize(countLines, sizeof(bool));
	for (int i = 0; i < countLines ; ++i)
	{
		m_inBox[i] = 0;
	}

	m_pointArray = new float[countPoints*3];
	m_colorArray = new float[countPoints*3];
	m_normalArray = new float[countPoints*3];
	m_lineArray = new int[lengthLines*4];
	m_lengthPoints = countPoints*3;
	m_lengthLines = lengthLines;

	dataFile.Seek(pc);
	dataFile.Read(m_pointArray, (size_t) countPoints*12);
	dataFile.Seek(lc);
	dataFile.Read(m_lineArray, (size_t) lengthLines*4);
	/*
	 * we don't use the color info saved here but calculate our own
	 *
	dataFile.Seek(cc);
	dataFile.Read(curves->m_colorArray, (size_t) countPoints*3);
	*/

	toggleEndianess();
	TheDataset::printTime();
	printf("move vertices\n");
	int xOff = TheDataset::columns/2;
	int yOff = TheDataset::rows/2;
	int zOff = TheDataset::frames/2;
	for (int i = 0; i < countPoints * 3 ; ++i) {
		m_pointArray[i] = xOff - m_pointArray[i];
		++i;
		m_pointArray[i] = m_pointArray[i] - yOff;
		++i;
		m_pointArray[i] = zOff - m_pointArray[i];
	}
	calculateLinePointers();
	createColorArray();
	TheDataset::printTime();
	printf("read all\n");

	m_type = Curves_;
	m_fullPath = filename;
	m_name = filename.AfterLast('/');
	initializeBuffer();

	m_kdTree = new KdTree(m_pointCount, m_pointArray);

	return true;
}


int Curves::getPointsPerLine(int line)
{
	return (m_linePointers[line+1] - m_linePointers[line]) ;
}

int Curves::getStartIndexForLine(int line)
{
	return m_linePointers[line];
}


void Curves::calculateLinePointers()
{
	TheDataset::printTime();
	printf("calculate line pointers\n");
	int pc = 0;
	int lc = 0;
	int tc = 0;
	for (int i = 0 ; i < m_lineCount ; ++i)
	{
		m_linePointers[i] = tc;
		lc = m_lineArray[pc];
		tc += lc;
		pc += (lc + 1);
	}

	lc = 0;
	pc = 0;


	for ( int i = 0 ; i < m_pointCount ; ++i)
	{
		if ( i == m_linePointers[lc+1]) ++lc;
		m_reverse[i] = lc;
	}
}

int Curves::getLineForPoint(int point)
{
	return m_reverse[point];
}

void Curves::toggleEndianess()
{
	TheDataset::printTime();
	printf("toggle Endianess\n");

	wxUint8 *pointbytes = (wxUint8*)m_pointArray;
	wxUint8 temp;
	for ( int i = 0 ; i < m_lengthPoints*4; i +=4)
	{
		temp  = pointbytes[i];
		pointbytes[i] = pointbytes[i+3];
		pointbytes[i+3] = temp;
		temp  = pointbytes[i+1];
		pointbytes[i+1] = pointbytes[i+2];
		pointbytes[i+2] = temp;
	}

	wxUint8 *linebytes = (wxUint8*)m_lineArray;
	for ( int i = 0 ; i < m_lengthLines*4; i +=4)
	{
		temp  = linebytes[i];
		linebytes[i] = linebytes[i+3];
		linebytes[i+3] = temp;
		temp  = linebytes[i+1];
		linebytes[i+1] = linebytes[i+2];
		linebytes[i+2] = temp;
	}
}

void Curves::createColorArray()
{
	TheDataset::printTime();
	printf("create color arrays\n");

	int pc = 0;
    float r,g,b, rr, gg, bb;
    float x1,x2,y1,y2,z1,z2;
    float lastx, lasty, lastz;
    for ( int i = 0 ; i < getLineCount() ; ++i )
    {
    	//pc = getStartIndexForLine(i)*3;
        x1 = m_pointArray[pc];
        y1 = m_pointArray[pc+1];
        z1 = m_pointArray[pc+2];
        x2 = m_pointArray[pc + getPointsPerLine(i)*3 - 3];
        y2 = m_pointArray[pc + getPointsPerLine(i)*3 - 2];
        z2 = m_pointArray[pc + getPointsPerLine(i)*3 - 1];

        r = (x1) - (x2);
        g = (y1) - (y2);
        b = (z1) - (z2);
        if (r < 0.0) r *= -1.0 ;
        if (g < 0.0) g *= -1.0 ;
        if (b < 0.0) b *= -1.0 ;

        float norm = sqrt(r*r + g*g + b*b);
        r *= 1.0/norm;
        g *= 1.0/norm;
        b *= 1.0/norm;

        lastx = lasty = lastz = 0.0;

        for (int j = 0; j < getPointsPerLine(i) ; ++j )
        {
        	rr = lastx - m_pointArray[pc];
            gg = lasty - m_pointArray[pc+1];
            bb = lastz - m_pointArray[pc+2];
            lastx = m_pointArray[pc];
            lasty = m_pointArray[pc+1];
            lastz = m_pointArray[pc+2];
            if (rr < 0.0) rr *= -1.0 ;
            if (gg < 0.0) gg *= -1.0 ;
            if (bb < 0.0) bb *= -1.0 ;
            float norm = sqrt(rr*rr + gg*gg + bb*bb);
            rr *= 1.0/norm;
            gg *= 1.0/norm;
            bb *= 1.0/norm;

        	m_normalArray[pc] = rr;
        	m_normalArray[pc+1] = gg;
        	m_normalArray[pc+2] = bb;

        	m_colorArray[pc] = r;
	        m_colorArray[pc+1] = g;
	        m_colorArray[pc+2] = b;
	        pc += 3;
        }
    }

}

void Curves::resetLinesShown()
{
	for (int i = 0; i < m_lineCount ; ++i)
	{
		m_inBox[i] = 0;
	}
}

void Curves::updateLinesShown(std::vector<std::vector<SelectionBox*> > boxes)
{
	for (uint i = 0 ; i != boxes.size() ; ++i)
	{
		bool dirty = false;
		for (uint j = 0 ; j < boxes[i].size() ; ++j)
		{
			if (boxes[i][j]->isDirty()) dirty = true;
		}
		if (dirty)
		{
			boxes[i][0]->m_inBox = getLinesShown(boxes[i][0]);
			boxes[i][0]->notDirty();

			for (uint j = 1 ; j < boxes[i].size() ; ++j)
			{
				if  (boxes[i][j]->isDirty()) {
					boxes[i][j]->m_inBox = getLinesShown(boxes[i][j]);
					boxes[i][j]->notDirty();
				}
				if ( boxes[i][j]->m_isActive) {
					for (int k = 0 ; k <m_lineCount ; ++k)
					boxes[i][0]->m_inBox[k] = boxes[i][0]->m_inBox[k] & ( (boxes[i][j]->m_inBox[k] | boxes[i][j]->m_isNOT) &
																			!(boxes[i][j]->m_inBox[k] & boxes[i][j]->m_isNOT));
				}
			}
		}
		if (boxes[i][0]->colorChanged())
		{
			glBindBuffer(GL_ARRAY_BUFFER, m_bufferObjects[1]);
			float *colorData = (float *)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
			wxColour col = boxes[i][0]->getColor();
			TheDataset::printwxT(col.GetAsString());
			printf("\n");

			for ( int l = 0 ; l < m_lineCount ; ++l )
			{
				if (boxes[i][0]->m_inBox[l])
				{
					unsigned int pc = getStartIndexForLine(l)*3;

					for (int j = 0; j < getPointsPerLine(l) ; ++j )
					{
						colorData[pc] = ((float)col.Red())/255.0;
						colorData[pc+1] = ((float)col.Green())/255.0;
						colorData[pc+2] = ((float)col.Blue())/255.0;
						pc += 3;
					}
				}
			}




			glUnmapBuffer(GL_ARRAY_BUFFER);
			boxes[i][0]->setColorChanged(false);
		}
	}
	resetLinesShown();
	for (uint i = 0 ; i < boxes.size() ; ++i)
	{
		if ( boxes[i][0]->m_isActive) {
			for (int k = 0 ; k <m_lineCount ; ++k)
				m_inBox[k] = m_inBox[k] | boxes[i][0]->m_inBox[k];
		}
	}
}

std::vector<bool> Curves::getLinesShown(SelectionBox* box)
{
	Vector3fT vpos = box->getCenter();
	Vector3fT vsize = box->getSize();
	resetLinesShown();
	m_boxMin = new float[3];
	m_boxMax = new float[3];
	m_boxMin[0] = vpos.s.X - vsize.s.X/2;
	m_boxMax[0] = vpos.s.X + vsize.s.X/2;
	m_boxMin[1] = vpos.s.Y - vsize.s.Y/2;
	m_boxMax[1] = vpos.s.Y + vsize.s.Y/2;
	m_boxMin[2] = vpos.s.Z - vsize.s.Z/2;
	m_boxMax[2] = vpos.s.Z + vsize.s.Z/2;

	boxTest(0, m_pointCount-1, 0);
	return m_inBox;
}

void Curves::boxTest(int left, int right, int axis)
{
	// abort condition
	if (left > right) return;

	int root = left + ((right-left)/2);
	int axis1 = (axis+1) % 3;
	int pointIndex = m_kdTree->m_tree[root]*3;

	if (m_pointArray[pointIndex + axis] < m_boxMin[axis]) {
		boxTest(root +1, right, axis1);
	}
	else if (m_pointArray[pointIndex + axis] > m_boxMax[axis]) {
		boxTest(left, root-1, axis1);
	}
	else {
		int axis2 = (axis+2) % 3;
		if (	m_pointArray[pointIndex + axis1] <= m_boxMax[axis1] &&
				m_pointArray[pointIndex + axis1] >= m_boxMin[axis1] &&
				m_pointArray[pointIndex + axis2] <= m_boxMax[axis2] &&
				m_pointArray[pointIndex + axis2] >= m_boxMin[axis2] )
		{
			m_inBox[getLineForPoint(m_kdTree->m_tree[root])] = 1;
		}
		boxTest(left, root -1, axis1);
		boxTest(root+1, right, axis1);
	}
}

void Curves::initializeBuffer()
{
	glGenBuffers(3, m_bufferObjects);
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferObjects[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*m_pointCount*3, m_pointArray, GL_STATIC_DRAW );
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferObjects[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*m_pointCount*3, m_colorArray, GL_STATIC_DRAW );
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferObjects[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*m_pointCount*3, m_normalArray, GL_STATIC_DRAW );
	freeArrays();

}

void Curves::draw()
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferObjects[0]);
	glVertexPointer(3, GL_FLOAT, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferObjects[1]);
	glColorPointer (3, GL_FLOAT, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferObjects[2]);
	glNormalPointer (GL_FLOAT, 0, 0);
	for ( int i = 0 ; i < m_lineCount ; ++i )
	{
		if (m_inBox[i] == 1)
			glDrawArrays(GL_LINE_STRIP, getStartIndexForLine(i), getPointsPerLine(i));
	}
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
}

FVector Curves::getBarycenter(FVector box)
{
	m_boxMin = new float[3];
	m_boxMax = new float[3];
	m_boxMin[0] = box[0] - box[3]/2;
	m_boxMax[0] = box[0] + box[3]/2;
	m_boxMin[1] = box[1] - box[4]/2;
	m_boxMax[1] = box[1] + box[4]/2;
	m_boxMin[2] = box[2] - box[5]/2;
	m_boxMax[2] = box[2] + box[5]/2;
	m_barycenter.clear();
	m_barycenter.resize(3, false);
	m_count = 0;

	barycenterTest(0, m_pointCount-1, 0);
	if (m_count > 0) {
	m_barycenter[0] /= m_count;
	m_barycenter[1] /= m_count;
	m_barycenter[2] /= m_count;
	}
	else {
		m_barycenter[0] = box[0];
		m_barycenter[1] = box[1];
		m_barycenter[2] = box[2];

	}
	return m_barycenter;
}

void Curves::barycenterTest(int left, int right, int axis)
{
	// abort condition
	if (left > right) return;

	int root = left + ((right-left)/2);
	int axis1 = (axis+1) % 3;
	int pointIndex = m_kdTree->m_tree[root]*3;

	if (m_pointArray[pointIndex + axis] < m_boxMin[axis]) {
		barycenterTest(root +1, right, axis1);
	}
	else if (m_pointArray[pointIndex + axis] > m_boxMax[axis]) {
		barycenterTest(left, root-1, axis1);
	}
	else {
		int axis2 = (axis+2) % 3;
		if (	m_inBox[getLineForPoint(m_kdTree->m_tree[root])] == 1 &&
				m_pointArray[pointIndex + axis1] <= m_boxMax[axis1] &&
				m_pointArray[pointIndex + axis1] >= m_boxMin[axis1] &&
				m_pointArray[pointIndex + axis2] <= m_boxMax[axis2] &&
				m_pointArray[pointIndex + axis2] >= m_boxMin[axis2] )
		{
			m_barycenter[0] += m_pointArray[m_kdTree->m_tree[root]*3];
			m_barycenter[1] += m_pointArray[m_kdTree->m_tree[root]*3+1];
			m_barycenter[2] += m_pointArray[m_kdTree->m_tree[root]*3+2];
			m_count++;
		}
		barycenterTest(left, root -1, axis1);
		barycenterTest(root+1, right, axis1);
	}
}
