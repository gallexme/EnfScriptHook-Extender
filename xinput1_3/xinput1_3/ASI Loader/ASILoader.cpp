#include "ASILoader.h"
#include "..\Utility\Log.h"
#include "..\Utility\General.h"
#include "..\Utility\PEImage.h"
#include <stdio.h>
using namespace Utility;
void getAsisDir( std::string dirn, std::vector<std::string>* asis)
{
	char dirnPath[1024];
	sprintf(dirnPath, "%s\\*", dirn.c_str());
	printf("Searching in :%s \n", dirnPath);
	WIN32_FIND_DATA f;
	HANDLE h = FindFirstFile(dirnPath, &f);

	if (h == INVALID_HANDLE_VALUE) { return; }
	
	do
	{
		const char * name = f.cFileName;

		if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) { continue; }
		
		char filePath[1024];
		sprintf(filePath, "%s%s%s", dirn.c_str(), "\\", name);
		
		
		if (f.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
		{
			getAsisDir(filePath,asis);
		}
		else {
			printf("found File:%s \n", name);
			asis->push_back(std::string(filePath));
		}

	} while (FindNextFile(h, &f));
	FindClose(h);
}
typedef void(__stdcall *f_registerScripts)();
void ASILoader::Initialize() {
	std::vector<std::string> asis = std::vector<std::string>();
	
	LOG_PRINT( "Loading *.dll EnfExtensions" );

	const std::string currentFolder = GetRunningExecutableFolder();
	const std::string asiFolder = currentFolder + "\\extensions";

	const std::string asiSearchQuery = asiFolder + "\\*.dll";
	
	getAsisDir(asiFolder, &asis);
	for (auto p : asis) {
		std::string::size_type idx;

		idx = p.rfind('.');

		if (idx != std::string::npos)
		{
			std::string extension = p.substr(idx + 1);


			if (extension == "dll" || extension == ".dll") {

				LOG_PRINT("Loading \"%s\"", p.c_str());

				PEImage pluginImage;
				if (!pluginImage.Load(p)) {

					LOG_ERROR("\tFailed to load image");
					continue;
				}

				if (!pluginImage.IsEnfHookCompatible()) {

					LOG_PRINT("\tDetected non compatible image");

					
				}

				// Image compatible (now), load it
				HMODULE module = LoadLibraryA(p.c_str());
				if (module) {
					LOG_PRINT("\tLoaded \"%s\" => 0x%p", p.c_str(), module);

					
					/*// resolve function address here
					f_funci funci = (f_funci)GetProcAddress(module, "funci");
					if (!funci) {
						std::cout << "could not locate the function" << std::endl;
						return EXIT_FAILURE;
					}*/
				}
				else {
					LOG_ERROR("\tFailed to load");
				}

			}
		}
		else {
			LOG_PRINT("NotLoading  \"%s\" ", p);
		}

	}
	LOG_PRINT( "Finished loading *.asi plugins" );
}
