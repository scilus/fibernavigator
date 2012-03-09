#ifndef LOADER_H_
#define LOADER_H_

#include "DatasetInfo.h"
#include "DatasetManager.h"
#include "../Logger.h"
#include "../gui/ListCtrl.h"
#include "../gui/MainFrame.h"
#include "../gui/SceneManager.h"

#include <wx/file.h>
#include <wx/string.h>

class Loader
{
private:
    MainFrame *m_pMainFrame;
    ListCtrl *m_pListCtrl;
    bool m_error;
public:
    Loader( MainFrame *pMainFrame, ListCtrl *pListCtrl ) 
    :   m_pMainFrame( pMainFrame ),
        m_pListCtrl( pListCtrl ),
        m_error( false )
    {
    }

    bool getError() { return m_error; }

    void operator()( const wxString &filename )
    {
        // check if i_fileName is valid
        if( !wxFile::Exists( filename ) )
        {
            Logger::getInstance()->print( wxString::Format( wxT( "File %s doesn't exist!" ), filename.c_str() ), LOGLEVEL_ERROR );
        }
        else
        {
            // If the file is in compressed formed, we check what kind of file it is
            wxString extension = filename.AfterLast( '.' );
            if( wxT( "gz" ) == extension )
            {
                extension = filename.BeforeLast( '.' ).AfterLast( '.' );
            }

            if( wxT( "scn" ) == extension )
            {
                if( !SceneManager::getInstance()->load( filename ) )
                {
                    m_error = true;
                }
            }
            else
            {
                int result = DatasetManager::getInstance()->load( filename, extension );
                if( -1 != result )
                {
                    DatasetInfo *pDataset = DatasetManager::getInstance()->getDataset( result );

                    switch( pDataset->getType() )
                    {
                        case HEAD_BYTE:
                        case HEAD_SHORT:
                        case OVERLAY:
                        case RGB:
                        {
                            if( 1 == DatasetManager::getInstance()->getAnatomyCount() )
                            {
                                m_pMainFrame->updateSliders();
                            }
                            break;
                        }
                        case FIBERS:
                        {
                            if( !DatasetManager::getInstance()->isFibersGroupLoaded() )
                            {
                                int result = DatasetManager::getInstance()->createFibersGroup();
                                m_pListCtrl->InsertItem( result );
                            }
                            break;
                        }
                        default:
                            break;
                    }

                    m_pListCtrl->InsertItem( result );
                }
                else
                {
                    m_error = true;
                }
            }
        }
    }
};

#endif // LOADER_H_
