#include "Linker.h"
#include "FbxObjects.h"
#include "Structs.h"


int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	VertexHeader Ver;
	FBX fbx;
	
	fbx.Initialization();
	
	return 0;
}