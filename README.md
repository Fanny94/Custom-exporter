# Custom-exporter
A project importing FBX files and exporting the data into a Custom format (.data)

This is a project developed by 4 Technical artist students. 

Desc;
The custom file format supports models created with Maya Autodesk, meshes including vertices, normals, UV:s and textures. The code can also import FBX data such as lights, cameras, morphanimation, skeleton animation adn write it to the custom format.

In the project's fbx_files folder a fbx file that contains a test mesh can be found (test.fbx). After compiling the code, a .data file will be created. 

Limations;
For this project to be compiled FBX SDK 2016.1 has to be installed on the computer.
To be able to import the FBX file the whole filepath for where the FBX is saved must be typed in the LoadScene function, for example:

bool ImporterStat = MyImporter->Initialize("C:/Users/funne/OneDrive/Dokument/FBX-Exporter/fbx_files/test.fbx", -1, PManager->GetIOSettings());



