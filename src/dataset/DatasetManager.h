#ifndef DATASETMANAGER_H_
#define DATASETMANAGER_H_

#include "../dataset/Anatomy.h"
#include "../misc/nifti/nifti1_io.h"

#include <map>
#include <set>
#include <vector>
#include <wx/string.h>

class DatasetHelper;
class DatasetInfo;
class Fibers;
class ODFs;
class Tensors;

class DatasetManager
{
public:
    ~DatasetManager(void);

    static DatasetManager * getInstance();

    DatasetInfo *           getDataset( int index );
    std::vector<Fibers *>   getFibers();
    unsigned int            getFibersCount()            { return m_fibers.size(); }
    std::vector<ODFs *>     getOdfs();
    bool                    isAnatomyLoaded()           { return !m_anatomies.empty(); }
    bool                    isOdfsLoaded()              { return !m_odfs.empty(); }

    // -1 if load unsuccessful, index of the dataset otherwise
    int load( const wxString &filename, const wxString &extension );

    int createAnatomy()                                                 { return insert( new Anatomy( m_pDatasetHelper ) ); }
    int createAnatomy( DatasetType type )                               { return insert( new Anatomy( m_pDatasetHelper, type ) ); }
    int createAnatomy( vector<float> *pDataset, DatasetType type )      { return insert( new Anatomy( m_pDatasetHelper, pDataset, type ) ); }

    // temporary
    void setDatasetHelper( DatasetHelper * dh );

protected:
    DatasetManager(void);

private:
    DatasetManager(const DatasetManager &);
    DatasetManager &operator=(const DatasetManager &);

    // Gets the next available index when loading new datasets
    int getNextAvailableIndex();

    // Inserts an anatomy into m_datasets and m_anatomies
    int insert( Anatomy * pAnatomy );
    int insert( ODFs * pOdfs );

    // Loads an anatomy. Extension supported: .nii and .nii.gz
    int loadAnatomy( const wxString &filename, nifti_image *pHeader, nifti_image *pBody );

    // Loads a fiber set. Extension supported: .fib, .bundlesdata, .trk and .tck
    int loadFibers( const wxString &filename );

    // Loads a mesh. Extension supported: .mesh, .surf and .dip
    int loadMesh( const wxString &filename );

    // Loads an ODF. Extension supported: .nii and .nii.gz
    int loadODF( const wxString &filename, nifti_image *pHeader, nifti_image *pBody );

    // Loads tensors. Extension supported: .nii and .nii.gz
    int loadTensors( const wxString &filename );

public:
    // temporary
    DatasetHelper *m_pDatasetHelper;

private:
    static DatasetManager *m_pInstance;

    unsigned int m_maxIndex;
    std::set<unsigned int> m_freeIndexes;

    std::map<unsigned int, DatasetInfo *> m_datasets;
    std::map<unsigned int, Anatomy *> m_anatomies;
    std::map<unsigned int, Fibers *> m_fibers;
    std::map<unsigned int, ODFs *> m_odfs;
    std::map<unsigned int, Tensors *> m_tensors;
};

#endif DATASETMANAGER_H_