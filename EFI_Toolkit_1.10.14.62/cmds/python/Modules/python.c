/* Minimal main program -- everything is loaded from the library */

#ifdef EFI_LOADABLE_MODULE
#undef EFI_LOADABLE_MODULE
#endif

#include "Python.h"

extern DL_EXPORT(int) Py_Main();

int
main(argc, argv)
	int argc;
	char **argv;
{
	return Py_Main(argc, argv);
}
