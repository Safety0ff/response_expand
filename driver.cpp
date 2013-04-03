#include <iostream>
#include "response.h"

int main(int argc, char** argv)
{
	size_t ulargc = argc;
	int retval = response_expand(&ulargc, &argv);
	std::cout << "pre-expand argc: " << argc << " post: " << ulargc << std::endl;
	std::cout << "return value: " << retval << std::endl;
	std::cout << "Post-expand arguments:\n----------------------\n";
	for (size_t i = 0; i < ulargc; ++i)
	{
		std::cout << i << ":\t__" << argv[i] << "__" << std::endl;
	}
	return retval;
}
