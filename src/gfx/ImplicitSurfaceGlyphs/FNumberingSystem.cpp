#include <iostream>
#include <vector>
//#include "common/src/stdAliases.hh"
#include "FNumberingSystem.h"

//#define TEST

#ifdef TEST
struct UCTest{
	UCTest()
	{
		unique_count uc(3,4);
		const int s = uc.nbUnique();
		for(int i=0; i< s; ++i, ++uc)
		{
			std::cout << "TEST:" << i << " " << uc() << "     ";
			std::vector<int> hist = uc.histogram();
			std::cout << hist[0] << " " << hist[1] << " " << hist[2] << "        " <<
				uc[0] <<  " " << uc[1] << " " << uc[2] << " " << uc[3] << std::endl;
		}

		//exit(0);
	}
};

const UCTest uctest;

#endif

