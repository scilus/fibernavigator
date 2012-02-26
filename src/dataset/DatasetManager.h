#ifndef DATASETMANAGER_H_
#define DATASETMANAGER_H_

#include "Anatomy.h"
#include "FibersGroup.h"
#include "ODFs.h"
#include "Surface.h"
#include "../misc/Fantom/FMatrix.h"
#include "../misc/nifti/nifti1_io.h"
#include "../misc/IsoSurface/CIsoSurface.h"

#include <wx/string.h>

#include <map>
#include <vector>

class DatasetHelper;
class DatasetInfo;
class Fibers;
class Mesh;
class Tensors;

typedef unsigned int DatasetIndex;

class DatasetManager
{
public:
    ~DatasetManager(void);

    static DatasetManager * getInstance();

    std::vector<Anatomy *>  getAnatomies() const;
    size_t                  getAnatomyCount() const         { return m_anatomies.size(); }
    DatasetInfo *           getDataset( DatasetIndex index ) const;
    std::vector<Fibers *>   getFibers() const;
    FibersGroup *           getFibersGroup() const;
    size_t                  getFibersCount() const          { return m_fibers.size(); }
    std::vector<ODFs *>     getOdfs() const;
    Fibers *                getSelectedFibers( DatasetIndex index ) const;
    Surface *               getSurface() const;
    std::vector<Tensors *>  getTensors() const;

    int                     getColumns() const;
    int                     getFrames() const;
    int                     getRows() const;
    float                   getVoxelX() const;
    float                   getVoxelY() const;
    float                   getVoxelZ() const;
    FMatrix &               getNiftiTransform()             { return m_niftiTransform; }
    unsigned int            getCountFibers() const          { return m_countFibers; }

    bool                    isDatasetLoaded() const         { return !m_datasets.empty(); }
    bool                    isAnatomyLoaded() const         { return !m_anatomies.empty(); }
    bool                    isFibersLoaded() const          { return !m_fibers.empty(); }
    bool                    isFibersGroupLoaded() const     { return !m_fibersGroup.empty(); }
    bool                    isMeshLoaded() const            { return !m_meshes.empty(); }
    bool                    isOdfsLoaded() const            { return !m_odfs.empty(); }
    bool                    isSurfaceLoaded() const         { return !m_surfaces.empty(); }
    bool                    isTensorsLoaded() const         { return !m_tensors.empty(); }
    bool                    isTensorsFieldLoaded() const    { return false; }
    bool                    isVectorsLoaded() const         { return false; }

    void  setCountFibers( const unsigned int count )        { m_countFibers = count; }
    bool  isSurfaceDirty() const                            { return m_surfaceIsDirty; }
    void  setSurfaceDirty( const bool dirty )               { m_surfaceIsDirty = dirty; }

    // -1 if load unsuccessful, index of the dataset otherwise
    int load( const wxString &filename, const wxString &extension );

    // return index of the created dataset
    DatasetIndex createAnatomy()                                                 { return insert( new Anatomy( m_pDatasetHelper ) ); }
    DatasetIndex createAnatomy( DatasetType type )                               { return insert( new Anatomy( m_pDatasetHelper, type ) ); }
    DatasetIndex createAnatomy( const Anatomy * const pAnatomy )                 { return insert( new Anatomy( m_pDatasetHelper, pAnatomy ) ); }
    DatasetIndex createAnatomy( vector<float> *pDataset, DatasetType type )      { return insert( new Anatomy( m_pDatasetHelper, pDataset, type ) ); }
    DatasetIndex createCIsoSurface( Anatomy *pAnatomy )                          { return insert( new CIsoSurface( m_pDatasetHelper, pAnatomy ) ); }
    DatasetIndex createFibersGroup()                                             { return insert( new FibersGroup( m_pDatasetHelper ) ); }
    DatasetIndex createODFs()                                                    { return insert( new ODFs( m_pDatasetHelper ) ); }
    DatasetIndex createSurface()                                                 { return insert( new Surface( m_pDatasetHelper ) ); }

    void remove( const DatasetIndex index );

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
    DatasetIndex getNextAvailableIndex() { return m_nextIndex++; }

    // Inserts the datasets in their corresponding maps
    DatasetIndex insert( Anatomy * pAnatomy );
    DatasetIndex insert( CIsoSurface * pCIsoSurface );
    DatasetIndex insert( Fibers * pFibers );
    DatasetIndex insert( FibersGroup * pFibersGroup );
    DatasetIndex insert( Mesh * pMesh );
    DatasetIndex insert( ODFs * pOdfs );
    DatasetIndex insert( Surface * pSurface );
    DatasetIndex insert( Tensors * pTensors );

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

    DatasetIndex m_nextIndex;

    std::map<DatasetIndex, DatasetInfo *> m_datasets;
    std::map<DatasetIndex, Anatomy *> m_anatomies;
    std::map<DatasetIndex, Fibers *> m_fibers;
    std::map<DatasetIndex, FibersGroup *> m_fibersGroup;
    std::map<DatasetIndex, Mesh *> m_meshes;
    std::map<DatasetIndex, ODFs *> m_odfs;
    std::map<DatasetIndex, Surface *> m_surfaces;
    std::map<DatasetIndex, Tensors *> m_tensors;

    FMatrix m_niftiTransform;
    unsigned int m_countFibers; // TODO: Remove me once selection is fixed
    bool m_surfaceIsDirty;
};

#endif DATASETMANAGER_H_