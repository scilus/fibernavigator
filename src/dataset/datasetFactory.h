/*
 * datasetFactory.h
 *
 *  Created on: 03/12/2010
 *      Author: GGirard
 */

#ifndef DATASETFACTORY_H_
#define DATASETFACTORY_H_

class DatasetHelper;
class DatasetInfo;
class wxString;

class DatasetFactory
{
    DatasetFactory(DatasetHelper *datasetHelper);
    DatasetInfo* loadNifti( wxString i_fileName );

private:
    DatasetHelper *m_dh;
};


#endif //DATASETFACTORY_H_