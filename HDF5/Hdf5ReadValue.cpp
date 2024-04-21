#include "Tools.h"
#include "Hdf5Function.h"
using namespace std;




/*��HDF5�ļ�*/
void Hdf5ReadValue::OpenFile(const char* filepath)
{
	this->file = H5Fopen(filepath, H5F_ACC_RDONLY, H5P_DEFAULT);
}

/*��HDF5�ļ���Ӧ��Group*/
void Hdf5ReadValue::OpenGroup(const char* group)
{
	this->group = H5Gopen2(this->file, group, H5P_DEFAULT);
}

/*��HDF5�ļ���Ӧ��Dataset*/
void Hdf5ReadValue::OpenDataset(const char* dataset)
{
	this->dataset = H5Dopen(this->group, dataset, H5P_DEFAULT);
}

/*���ļ��еĸ�����������int,double,string*/
void Hdf5ReadValue::ReadIntData(void *data)
{
	this->status = H5Dread(this->dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
}

void Hdf5ReadValue::ReadDoubleData(void *data)
{
	this->status = H5Dread(this->dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
}

void Hdf5ReadValue::ReadStringData(void *data)
{
	size_t size = sizeof(data) / sizeof(char);
	this->status = H5Tset_size(dtype, size);
	this->status = H5Dread(this->dataset, this->dtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
}

/*�ر����ݼ��ļ�*/
void Hdf5ReadValue::CloseFile()
{
	this->status = H5Dclose(this->dataset);
	this->status = H5Gclose(this->group);
	this->status = H5Fclose(this->file);
}