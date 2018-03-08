Instalation
===========

In order to use the Blender addon exporters for AeonEngine, you need to install the python protobuffer library onto your Blender setup because it is a pain to have Blender use the local python instalation. Either way you need to build and install the PB library somewhere.
Specifically, you will need to install pip and then build and install the PB library from source.

I may expand some more on this, but right now this is mostly something to look up when configuring an environment. The process is quite involved and I couldn't or haven't been able to easily automate it.

1. Run as administrator or root a command line window or terminal.
2. Add the directory containing the python executable from Blender into the PATH enviroment variable, this is usually Program Files -> Blender Foundation -> Blender Version -> python -> bin. (Only valid for Windows, Ubuntu 16.04 now uses the system's python, but you may have to install pip for python3 since the default is python 2.x).
3. Install pip using get-pip.py (https://pip.pypa.io/en/stable/installing/)
4. IF installing from source:
    1. After building and/or installing protocol buffer, add the path to protoc to the PATH enviroment variable.
    2. From the python folder under the protobuffer source run "python setup.py build" then "python setup.py test" and "python setup.py install"
5. IF installing from the PIP package directory:
    1. Run either 'python -m pip install protobuf' OR just 'pip install protobuf' at the time of writing, the version at pip is 3.1.0 which should be compatible with the one on the runtime.

Thats it. To actually run the exporters:

1. Make or build the generate-python-protobuf-source target for [this project](https://github.com/AeonGames/AeonEngine).
2. Run Blender and go to File->User Preferences and click on the "Files" Tab.
3. Type or browse for this folder on the "Scripts" field.
4. Save preferences and restart Blender.
5. Open the preferences window again and the exporter should be listed as an Import-Export Addon on the Addons tab.
6. Check the box to the left of the addon name/description, the exporter should report no errors during loading.
7. Go to File->Export and find the type you want to export to.

There is only export functionality for the time being, so avoid lossing the original blend file if you want to make changes to your model down the road.
