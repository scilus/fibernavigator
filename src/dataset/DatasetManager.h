#ifndef DATASETMANAGER_H_
#define DATASETMANAGER_H_

#include "Anatomy.h"
#include "DatasetIndex.h"
#include "FibersGroup.h"
#include "ODFs.h"
#include "Maximas.h"
#include "RestingStateNetwork.h"
#include "../misc/Fantom/FMatrix.h"
#include "../misc/nifti/nifti1_io.h"
#include "../misc/IsoSurface/CIsoSurface.h"

#include <wx/string.h>

#include <algorithm>
#include <map>
#include <vector>

class DatasetInfo;
class Fibers;
class Mesh;
class Tensors;

const DatasetIndex BAD_INDEX;

class DatasetManager
{
public:
    ~DatasetManager(void);

    static DatasetManager * getInstance();

    std::vector<Anatomy *>  getAnatomies() const;
    size_t                  getAnatomyCount() const         { return m_anatomies.size(); }
    DatasetInfo *           getDataset( DatasetIndex index ) const;
    size_t                  getDatasetCount() const         { return m_datasets.size(); }
    DatasetIndex            getDatasetIndex( DatasetInfo * pDatasetInfo ) const;
    std::vector<Fibers *>   getFibers() const;
    FibersGroup *           getFibersGroup() const;
    size_t                  getFibersCount() const          { return m_fibers.size(); }
    std::vector<Mesh *>     getMeshes() const;
    std::vector<ODFs *>     getOdfs() const;
    std::vector<Maximas *>  getMaximas() const;
    Fibers *                getSelectedFibers( DatasetIndex index ) const;
    std::vector<Tensors *>  getTensors() const;
    RestingStateNetwork     *m_pRestingStateNetwork;

    int                     getColumns() const;
    int                     getFrames() const;
    int                     getRows() const;
    int                     getBands() const;
    float                   getVoxelX() const;
    float                   getVoxelY() const;
    float                   getVoxelZ() const;
    FMatrix &               getNiftiTransform()             { return m_niftiTransform; }

    bool                    isDatasetLoaded() const         { return !m_datasets.empty(); }
    bool                    isAnatomyLoaded() const         { return !m_anatomies.empty(); }
    bool                    isFibersLoaded() const          { return !m_fibers.empty(); }
    bool                    isFibersGroupLoaded() const     { return !m_fibersGroup.empty(); }
    bool                    isMeshLoaded() const            { return !m_meshes.empty(); }
    bool                    isOdfsLoaded() const            { return !m_odfs.empty(); }
    bool                    isTensorsLoaded() const         { return !m_tensors.empty(); }
    bool                    isMaximasLoaded() const         { return !m_maximas.empty(); }
    bool                    isTensorsFieldLoaded() const    { return false; }
    bool                    isVectorsLoaded() const         { return false; }
    
    void                    forceLoadingAsMaximas(bool force) { m_forceLoadingAsMaximas = force; }
    void                    forceLoadingAsRestingState(bool force) { m_forceLoadingAsRestingState = force; }

    void clear();

    // Check with DatasetIndex::isOk() method to know if index is valid
    DatasetIndex load( const wxString &filename, const wxString &extension );

    // return index of the created dataset
    DatasetIndex createAnatomy()                                                 { return insert( new Anatomy() ); }
    DatasetIndex createAnatomy( DatasetType type )                               { return insert( new Anatomy( type ) ); }
    DatasetIndex createAnatomy( const Anatomy * const pAnatomy, bool offset )    { return insert( new Anatomy( pAnatomy, offset ) ); }
    // TODO remove
    //DatasetIndex createAnatomy( std::vector<float> *pDataset, int sample )       { return insert( new Anatomy( pDataset, sample ) ); }
    DatasetIndex createAnatomy( std::vector<float> *pDataset, int type )         { return insert( new Anatomy( pDataset, type ) ); }
    DatasetIndex createCIsoSurface( Anatomy *pAnatomy )                          { return insert( new CIsoSurface( pAnatomy ) ); }
    DatasetIndex createFibersGroup()                                             { return insert( new FibersGroup() ); }
    DatasetIndex createODFs( const wxString &filename )                          { return insert( new ODFs( filename ) ); }
    DatasetIndex createMaximas( const wxString &filename )                       { return insert( new Maximas( filename ) ); }

    void remove( const DatasetIndex index );
	DatasetIndex createFibers();
    DatasetIndex addFibers( Fibers* fibers );

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
    DatasetIndex insert( Tensors * pTensors );
    DatasetIndex insert( Maximas * pMaximas );

    // Loads an anatomy. Extension supported: .nii and .nii.gz
    DatasetIndex loadAnatomy( const wxString &filename, nifti_image *pHeader, nifti_image *pBody );

    // Loads a fiber set. Extension supported: .fib, .vtk, .bundlesdata, .trk and .tck
    DatasetIndex loadFibers( const wxString &filename );


    // Loads a mesh. Extension supported: .mesh, .surf and .dip
    DatasetIndex loadMesh( const wxString &filename, const wxString &extension );

    // Loads an ODF. Extension supported: .nii and .nii.gz
    DatasetIndex loadODF( const wxString &filename, nifti_image *pHeader, nifti_image *pBody );

    // Loads tensors. Extension supported: .nii and .nii.gz
    DatasetIndex loadTensors( const wxString &filename, nifti_image *pHeader, nifti_image *pBody );

    // Loads Maximas. Extension supported: .nii and .nii.gz
    DatasetIndex loadMaximas( const wxString &filename, nifti_image *pHeader, nifti_image *pBody );
    
    // Loads Resting-state fMRI. Extension supported: .nii and .nii.gz
    DatasetIndex loadRestingState( const wxString &filename, nifti_image *pHeader, nifti_image *pBody );
    
private:
    static DatasetManager *m_pInstance;

    DatasetIndex m_nextIndex;

    std::map<DatasetIndex, DatasetInfo *> m_datasets;
    std::map<DatasetIndex, Anatomy *> m_anatomies;
    std::map<DatasetIndex, Fibers *> m_fibers;
    std::map<DatasetIndex, FibersGroup *> m_fibersGroup;
    std::map<DatasetIndex, Mesh *> m_meshes;
    std::map<DatasetIndex, ODFs *> m_odfs;
    std::map<DatasetIndex, Tensors *> m_tensors;
    std::map<DatasetIndex, Maximas *> m_maximas;
    std::map<DatasetInfo *, DatasetIndex> m_reverseDatasets;

    FMatrix m_niftiTransform;
    
    bool m_forceLoadingAsMaximas;
    bool m_forceLoadingAsRestingState;
};

#endif //DATASETMANAGER_H_
