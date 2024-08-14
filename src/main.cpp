#include "first_app.hpp"

#include <cstdlib>
#include <iostream>
 

int main() {
	va::FirstApp app{};
	try {
		app.run();
		return EXIT_SUCCESS;
	}
	catch(const std::exception &e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
