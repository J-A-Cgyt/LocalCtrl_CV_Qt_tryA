#pragma once

#if MathActivate == 1

//第一个写一个高斯分布的随机数生成器，不过似乎opencv的RNG::Gaussian就可，既然如此就不浪费时间了到时后续肯定还会有更多方法需要数学计算，在这里写吧

//阶乘
double JieCheng(const unsigned int &i);

#endif

//看来就是按照源文件排列的顺序编译的 这个头文件完了以后是calib 这里没打分号直接后面报错 注意这种问题