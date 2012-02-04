#ifndef DATASETMANAGER_H_
#define DATASETMANAGER_H_

#include "Anatomy.h"
#include "FibersGroup.h"
#include "ODFs.h"
#include "Surface.h"
#include "../misc/nifti/nifti1_io.h"
#include "../misc/IsoSurface/CIsoSurface.h"

#include <map>
#include <vector>
#include <wx/string.h>

class DatasetHelper;
class DatasetInfo;
class Fibers;
class Mesh;
class Tensors;

class DatasetManager
{
public:
    ~DatasetManager(void);

    static DatasetManager * getInstance();

    DatasetInfo *           getDataset( unsigned int index );
    std::vector<Fibers *>   getFibers();
    unsigned int            getFibersCount() const { return m_fibers.size(); }
    std::vector<ODFs *>     getOdfs();

    bool                    isAnatomyLoaded() const         { return !m_anatomies.empty(); }
    bool                    isFibersLoaded() const          { return !m_fibers.empty(); }
    bool                    isFibersGroupLoaded() const     { return !m_fibersGroup.empty(); }
    bool                    isMeshLoaded() const            { return !m_meshes.empty(); }
    bool                    isOdfsLoaded() const            { return !m_odfs.empty(); }
    bool                    isSurfaceLoaded() const         { return !m_surfaces.empty(); }
    bool                    isTensorsLoaded() const         { return !m_tensors.empty(); }
    bool                    isTensorsFieldLoaded() const    { return false; }
    bool                    isVectorsLoaded() const         { return false; }


    // -1 if load unsuccessful, index of the dataset otherwise
    int load( const wxString &filename, const wxString &extension );

    // return index of the created dataset
    unsigned int createAnatomy()                                                 { return insert( new Anatomy( m_pDatasetHelper ) ); }
    unsigned int createAnatomy( DatasetType type )                               { return insert( new Anatomy( m_pDatasetHelper, type ) ); }
    unsigned int createAnatomy( vector<float> *pDataset )                        { return insert( new Anatomy( m_pDatasetHelper, pDataset ) ); }
    unsigned int createAnatomy( vector<float> *pDataset, DatasetType type )      { return insert( new Anatomy( m_pDatasetHelper, pDataset, type ) ); }
    unsigned int createCIsoSurface( Anatomy *pAnatomy )                          { return insert( new CIsoSurface( m_pDatasetHelper, pAnatomy ) ); }
    unsigned int createFibersGroup()                                             { return insert( new FibersGroup( m_pDatasetHelper ) ); }
    unsigned int createODFs()                                                    { return insert( new ODFs( m_pDatasetHelper ) ); }
    unsigned int createSurface()                                                 { return insert( new Surface( m_pDatasetHelper ) ); }

    void remove( const long ptr );

    // temporary
    void setDatasetHelper( DatasetHelper * dh );

    // Temp
    void updateLoadStatus() { }

protected:
    DatasetManager(void);

private:
    DatasetManager(const DatasetManager &);
    DatasetManager &operator=(const DatasetManager &);

    // Gets the next available index when loading new datasets
    int getNextAvailableIndex() { return m_nextIndex++; }

    // Inserts the datasets in their corresponding maps
    int insert( Anatomy * pAnatomy );
    int insert( CIsoSurface * pCIsoSurface );
    int insert( Fibers * pFibers );
    int insert( FibersGroup * pFibersGroup );
    int insert( Mesh * pMesh );
    int insert( ODFs * pOdfs );
    int insert( Surface * pSurface );
    int insert( Tensors * pTensors );

    // Loads an anatomy. Extension supported: .nii and .nii.gz
    int loadAnatomy( const wxString &filename, nifti_image *pHeader, nifti_image *pBody );

    // Loads a fiber set. Extension supported: .fib, .bundlesdata, .trk and .tck
    int loadFibers( const wxString &filename );

    // Loads a mesh. Extension supported: .mesh, .surf and .dip
    int loadMesh( const wxString &filename, const wxString &extension );

    // Loads an ODF. Extension supported: .nii and .nii.gz
    int loadODF( const wxString &filename, nifti_image *pHeader, nifti_image *pBody );

    // Loads tensors. Extension supported: .nii and .nii.gz
    int loadTensors( const wxString &filename, nifti_image *pHeader, nifti_image *pBody );
    
public:
    // temporary
    DatasetHelper *m_pDatasetHelper;

private:
    static DatasetManager *m_pInstance;

    unsigned int m_nextIndex;

    std::map<unsigned int, DatasetInfo *> m_datasets;
    std::map<unsigned int, Anatomy *> m_anatomies;
    std::map<unsigned int, Fibers *> m_fibers;
    std::map<unsigned int, FibersGroup *> m_fibersGroup;
    std::map<unsigned int, Mesh *> m_meshes;
    std::map<unsigned int, ODFs *> m_odfs;
    std::map<unsigned int, Surface *> m_surfaces;
    std::map<unsigned int, Tensors *> m_tensors;
};

#endif DATASETMANAGER_H_