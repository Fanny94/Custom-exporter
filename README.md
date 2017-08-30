# Custom-exporter
A project converting data from FBX files and exporting the data into a Custom format (.data)

This is a project developed by 4 Technical artist students. 

Desc:
The custom file format supports models created with Maya Autodesk; meshes including its vertices, normals, UV:s and textures. The project can also import lights, cameras, morphanimation and skeleton animation, and write it to the custom format.

A fbx file that contains a test mesh can be found (test.fbx) in the project's fbx_files folder. After compiling the code, a .data file will be created. 

Limations:
For this project to be compiled FBX SDK 2016.1 has to be installed on the computer.
The data that is exported into the custom format can only send one mesh and one texture per file.
