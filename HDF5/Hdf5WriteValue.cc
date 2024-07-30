
#include "Hdf5Function.h"
using namespace std;

/*创建HDF5*/
void Hdf5WriteValue::CreateNewFile(string fileName)
{
	this->file = H5Fcreate(fileName.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
	//H5F_ACC_TRUNC能覆盖，H5F_ACC_EXCL不能覆盖

}

/*创建HDF5 String数据维度*/
void Hdf5WriteValue::CreateStringDataspace(const void *data, int rank, int col, int row)
{
	hsize_t dim[2];
	dim[0] = row;
	dim[1] = col;
	size_t size = sizeof(data) / sizeof(char);
	this->status = H5Tset_size(dtype, size);
	this->dataspace = H5Screate_simple(rank, dim, NULL);
}

/*创建HDF5数据维度*/
void Hdf5WriteValue::CreateDataspace(int rank, int col, int row)
{
	hsize_t dim[2];
	dim[0] = row;
	dim[1] = col;
	this->dataspace = H5Screate_simple(rank, dim, NULL);
}

/*创建group*/
void Hdf5WriteValue::CreateGroup(string groupName)
{
	this->group = H5Gcreate(this->file, groupName.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
}

/*创建dataset,int,double,string*/
void Hdf5WriteValue::CreateIntDataset(string datasetName)
{
	this->dataset = H5Dcreate2(this->group, datasetName.c_str(), H5T_NATIVE_INT, this->dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
}

void Hdf5WriteValue::CreateDoubleDataset(string datasetName)
{
	this->dataset = H5Dcreate2(this->group, datasetName.c_str(), H5T_NATIVE_DOUBLE, this->dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
}

void Hdf5WriteValue::CreateStringDataset(string datasetName)
{
	this->dataset = H5Dcreate2(this->group, datasetName.c_str(), this->dtype, this->dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
}

/*写入数据到对应类型的dataset*/
void Hdf5WriteValue::WriteIntValue(const void *data)
{
	this->status = H5Dwrite(this->dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
}

void Hdf5WriteValue::WriteDoubleValue(const void *data)
{
	this->status = H5Dwrite(this->dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
}

void Hdf5WriteValue::WriteStringValue(const void *data)
{
	this->status = H5Dwrite(this->dataset, this->dtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
}
/*关闭数据及文件，注意关闭顺序*/
void Hdf5WriteValue::CloseFile()
{
	this->status = H5Dclose(this->dataset);
	this->status = H5Sclose(this->dataspace);
	this->status = H5Gclose(this->group);
	this->status = H5Fclose(this->file);
}