#include <windows.h>
#include <fstream>
#include "Hdf5Function.h"

int main()
{
	/*���Դ���*/
	//һάint
	const int a = 99;
	const int*value1 = &a;
	//һάdouble
	const double b = 99.99;
	const double* value2 = &b;
	//һάchar
	const char* value3 = "abc";
	//��άintle����
	int value4[5][6];
	int i, j;
	for (i = 0; i < 5; i++)
	{
		for (j = 0; j < 6; j++)
		{
			value4[i][j] = i + j;
		}
	}
	//��άdouble����
	double value5[289][28];
	int i, j;
	for (i = 0; i < 289; i++)
	{
		for (j = 0; j < 28; j++)
		{
			value5[i][j] = 0.1 + i + j;
		}
	}

	//��Ŷ����ݵ����ݽṹ
	double* result = new double[289 * 28];
	char* result1 = new char[20];

	//ͬһ���ļ���д����
	Hdf5WriteValue write;
	write.CreateNewFile("first.h5");
	//дһ��int����
	write.CreateDataspace(1, 0, 1);
	write.CreateGroup("groupA");
	write.CreateIntDataset("datasetA");
	write.WriteIntValue(value1);
	//дһ��double����
	write.CreateDoubleDataset("datasetB");
	write.WriteDoubleValue(value2);
	//�½�һ��groupдһ��string����
	write.CreateGroup("groupB");
	write.CreateStringDataspace(value3,1, 0, 1);
	write.CreateStringDataset("datasetC");
	write.WriteStringValue(value3);
	//дһ��int��ά����
	write.CreateDataspace(2, 5, 6);
	write.CreateIntDataset("datasetD");
	write.WriteIntValue(value4);
	//дһ��double��ά����
	write.CreateDataspace(2, 289, 28);
	write.CreateDoubleDataset("datasetE");
	write.WriteDoubleValue(value5);

	write.CloseFile();

	//��H5��HDF5�ļ�
	Hdf5ReadValue read;
	//��double����
	char filename[] = "hdf5.h5";
	read.OpenFile(filename);
	read.OpenGroup("groupB");
	read.OpenDataset("datasetE");
	read.ReadDoubleData(result);
	//��ӡ����
	cout.setf(ios::fixed, ios::floatfield);
	cout.precision(2);
	for (i = 0; i < 8092; i++)
	{
		cout << result[i] << endl;
	}

	//��ȡ�ַ�������
	read.OpenGroup("groupB");
	read.OpenDataset("datasetC");
	read.ReadStringData(result1);
	cout << result1 << endl;

	read.CloseFile();
	delete[] result;
	delete[] result1;
	system("pause");
	return 0;

}