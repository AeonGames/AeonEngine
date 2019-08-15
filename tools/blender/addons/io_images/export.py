# Copyright (C) 2017,2019 Rodrigo Jose Hernandez Cordoba
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import bpy
import os

class IMG_OT_exporter(bpy.types.Operator):

    '''Saves All Images in a specified directory'''
    bl_idname = "export_images.img"
    bl_label = "All Images"

    directory: bpy.props.StringProperty(subtype='DIR_PATH')

    @classmethod
    def poll(cls, context):
        return len(bpy.data.images) > 0

    def execute(self, context):
        if len(bpy.data.images) == 0:
            return {'CANCELLED'}
        for image in bpy.data.images:
            filepath = image.filepath
            image.filepath = self.directory + os.sep + os.path.basename(filepath)
            print("Saving ",image.filepath)            
            image.save()
            image.filepath = filepath
        print("Done.")
        return {'FINISHED'}

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {"RUNNING_MODAL"}
