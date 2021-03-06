//用于纯数学计算的函数实现 对应头文件MathCgyt.h
#include "Func_Proj_2nd.h"

#if MathActivate == 1

//第一个写个关于Zernike矩的计算
double JingXiang_R_n_m(unsigned int n, unsigned int m, double rho)
{
	if (n <= m)
	{
		cout << "n<=m is not allowed" << endl;
		return 0xffffffffffffffff;
	}

	//缺省返回
	return 0.0;
}

//阶乘 输入参数必须是int
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