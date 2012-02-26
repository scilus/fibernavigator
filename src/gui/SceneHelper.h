#include "SceneManager.h"
#include "../misc/IsoSurface/Vector.h"

#include <GL/glew.h>

//////////////////////////////////////////////////////////////////////////
// Some methods helping to map to and fro the scene
//////////////////////////////////////////////////////////////////////////

static Vector mapMouse2World( const int x, const int y, GLdouble projection[16], GLint viewport[4], GLdouble modelview[16] )
{
    glPushMatrix();
    SceneManager::getInstance()->doMatrixManipulation();

    GLfloat winX, winY;

    winX = (float) x;
    winY = (float) viewport[3] - (float) y;

    GLdouble posX, posY, posZ;
    gluUnProject( winX, winY, 0, modelview, projection, viewport, &posX, &posY, &posZ );
    glPopMatrix();

    return Vector( posX, posY, posZ );
}

static Vector mapMouse2WorldBack( const int x, const int y, GLdouble projection[16], GLint viewport[4], GLdouble modelview[16] )
{   
    GLfloat winX, winY;

    winX = (float) x;
    winY = (float) viewport[3] - (float) y;

    GLdouble posX, posY, posZ;
    gluUnProject( winX, winY, 1, modelview, projection, viewport, &posX, &posY, &posZ );

    return Vector( posX, posY, posZ );
}
