//���ڴ���ѧ����ĺ���ʵ�� ��Ӧͷ�ļ�MathCgyt.h
#include "Func_Proj_2nd.h"

#if MathActivate == 1

//��һ��д������Zernike�صļ���
double JingXiang_R_n_m(unsigned int n, unsigned int m, double rho)
{
	if (n <= m)
	{
		cout << "n<=m is not allowed" << endl;
		return 0xffffffffffffffff;
	}

	//ȱʡ����
	return 0.0;
}

//�׳� �������������int
double JieCheng(const unsigned int &i)  
{
	int Res = 1;

	if (i == 0)
	{
		return (double)Res;
	}
	else {	
		int para = i;
		while (para)
		{
			Res = Res * para;
			para = para - 1;
		}
		return (double)Res;
	}
}

#endif