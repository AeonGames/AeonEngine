# Copyright (C) 2026 Rodrigo Jose Hernandez Cordoba
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
"""Retarget Synty SIDEKICK animation FBX files onto a character rig.

The Sidekick animations are authored on the full 121 bone rig while the parts
are skinned to an 88 bone subset of it. AEONANM frames store bones positionally,
so the clip has to be sampled from the same armature the skeleton was exported
from; importing the animation on its own would emit the authoring rig's order.

Bone names match between the two rigs but their rest poses do not (the pelvis
sits ~3.8 units apart, finger tips ~66), and pose fcurves are stored relative to
the rest pose, so simply assigning the action to the character armature bends it
into a mess. Each character bone is instead constrained to its counterpart on
the clip rig and the result baked with visual keying, which resolves through
world space and is immune to the rest pose difference.

The skeleton and animation exporters read armature local space while the mesh
exporter bakes the world matrix, so the rig's 0.01 FBX object scale is applied
first; otherwise the clip comes out in centimetres against metre meshes.

Run headless from the repository root:

    blender -b --factory-startup -noaudio --python tools/blender/sidekick_anim.py -- \\
        --rig <a part .fbx> --out game/sidekick/knight/animations \\
        --anim Idle=<A_Body_IdleSubtle.fbx>

Use the same part the skeleton was exported from, so the baked bone order
matches the skeleton the animation is played against.

KNOWN LIMITATION: the body clips rest arms-down while the modular parts bind in
a T-pose, and that difference lives in the clip rig's rest pose rather than in
the animation, so a converted clip replays its motion from the character's own
bind pose -- the subtle idle reads as a breathing T-pose. Unity bridges this
with humanoid Avatar retargeting. Closing the gap here means rebinding the parts
to the clip rig's rest pose at cook time, or adding an avatar mapping layer.
The face clips share the parts' rest pose and are unaffected.
"""

import math
import os
import sys

import bpy


def _install_addon_path():
    root = os.path.dirname(os.path.abspath(__file__))
    if root not in sys.path:
        sys.path.insert(0, root)
    for folder in ("modules", "addons"):
        candidate = os.path.join(root, folder)
        if candidate not in sys.path:
            sys.path.insert(0, candidate)


def _parse_args(argv):
    if "--" in argv:
        argv = argv[argv.index("--") + 1:]
    else:
        argv = []
    options = {"rig": None, "out": None, "anims": [], "text": False, "rest_from": None}
    index = 0
    while index < len(argv):
        argument = argv[index]
        if argument == "--rig":
            index += 1
            options["rig"] = argv[index]
        elif argument == "--rest-from":
            index += 1
            options["rest_from"] = argv[index]
        elif argument in ("--out", "-o"):
            index += 1
            options["out"] = argv[index]
        elif argument == "--anim":
            index += 1
            if "=" not in argv[index]:
                raise SystemExit("--anim expects <name>=<file.fbx>, got {}".format(argv[index]))
            name, _, path = argv[index].partition("=")
            options["anims"].append((name, path))
        elif argument == "--text":
            options["text"] = True
        else:
            raise SystemExit("Unknown option {}".format(argument))
        index += 1
    if not options["rig"] or not options["out"] or not options["anims"]:
        raise SystemExit(
            "Usage: --rig <part.fbx> --out <dir> --anim <name>=<clip.fbx> [--anim ...] [--text]")
    return options


def _retarget(rig, clip_rig, name):
    """Resample the clip onto the character rig, preserving each bone's rest offset.

    Copying the clip bone's world transform outright only works where both rigs
    share a rest orientation. They do for the spine and legs but not the arms or
    fingers, and since the meshes are bound to this rig's rest pose the mismatch
    shows up as stretched limbs. Applying the constant offset between the two
    rest poses is what makes the deformation correct.
    """
    action = clip_rig.animation_data.action
    frame_start = int(action.frame_range[0])
    frame_end = int(action.frame_range[1])
    clip_bones = {b.name for b in clip_rig.data.bones}
    shared = [b.name for b in rig.data.bones if b.name in clip_bones]

    rig_world = rig.matrix_world
    rig_world_inverted = rig_world.inverted()
    clip_world = clip_rig.matrix_world
    offsets = {}
    for bone_name in shared:
        clip_rest = clip_world @ clip_rig.data.bones[bone_name].matrix_local
        rig_rest = rig_world @ rig.data.bones[bone_name].matrix_local
        # Full rest offset, so the clip's motion is replayed as a delta from this
        # rig's own bind pose. Forcing the clip's absolute bone positions instead
        # cannot be satisfied by a hierarchy with different bone offsets: the
        # basis matrices pick up shear that does not survive the exporter's TRS
        # decomposition and the character collapses.
        offsets[bone_name] = clip_rest.inverted() @ rig_rest

    if rig.animation_data is None:
        rig.animation_data_create()
    rig.animation_data.action = bpy.data.actions.new(name)
    for bone_name in shared:
        rig.pose.bones[bone_name].rotation_mode = 'QUATERNION'

    for frame in range(frame_start, frame_end + 1):
        bpy.context.scene.frame_set(frame)
        # The action is evaluated into the depsgraph copy; the original
        # datablock's pose still reads as the rest pose.
        clip_eval = clip_rig.evaluated_get(bpy.context.evaluated_depsgraph_get())
        posed = {}
        for bone_name in shared:
            clip_pose = clip_world @ clip_eval.pose.bones[bone_name].matrix
            posed[bone_name] = rig_world_inverted @ clip_pose @ offsets[bone_name]
        for bone_name in shared:
            bone = rig.data.bones[bone_name]
            pose_bone = rig.pose.bones[bone_name]
            if bone.parent is None or bone.parent.name not in posed:
                pose_bone.matrix_basis = bone.matrix_local.inverted() @ posed[bone_name]
            else:
                rest_delta = bone.parent.matrix_local.inverted() @ bone.matrix_local
                pose_bone.matrix_basis = (rest_delta.inverted() @
                                          posed[bone.parent.name].inverted() @
                                          posed[bone_name])
            pose_bone.keyframe_insert('location', frame=frame)
            pose_bone.keyframe_insert('rotation_quaternion', frame=frame)
            pose_bone.keyframe_insert('scale', frame=frame)

    print("{}: resampled frames {}..{} over {} bones".format(
        name, frame_start, frame_end, len(shared)))
    return rig.animation_data.action


def main():
    _install_addon_path()
    import importlib
    importlib.import_module("io_animation_anm").register()

    options = _parse_args(sys.argv)
    os.makedirs(options["out"], exist_ok=True)

    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=options["rig"])
    rig = next((o for o in bpy.data.objects if o.type == 'ARMATURE'), None)
    if rig is None:
        raise SystemExit("No armature in {}".format(options["rig"]))
    # Same scale and facing correction sidekick_batch applies, so the clip lands
    # in the space the skeleton and meshes were cooked into.
    from sidekick_batch import normalize_rig, _rebind_to_rest
    normalize_rig(list(bpy.data.objects))
    if options["rest_from"]:
        # Match however the parts were cooked, or the clip is sampled against a
        # rest pose the skeleton no longer has.
        mesh = next((o for o in bpy.data.objects if o.type == 'MESH'), None)
        _rebind_to_rest(rig, mesh, options["rest_from"])

    baked_actions = []
    for name, path in options["anims"]:
        before = {o.name for o in bpy.data.objects}
        bpy.ops.import_scene.fbx(filepath=path)
        imported = [o for o in bpy.data.objects if o.name not in before]
        clip_rig = next((o for o in imported if o.type == 'ARMATURE'), None)
        if clip_rig is None or clip_rig.animation_data is None:
            raise SystemExit("No animated armature in {}".format(path))
        # Turn the clip rig to face the same way as the cooked rig. Rotating the
        # object rather than applying it keeps the action, which is stored
        # relative to the clip's own rest pose, untouched.
        clip_rig.rotation_euler.z += math.pi
        bpy.context.view_layer.update()
        baked_actions.append(_retarget(rig, clip_rig, name))
        for obj in imported:
            bpy.data.objects.remove(obj, do_unlink=True)

    # The exporter walks bpy.data.actions, so the source clips must not survive
    # or they would be written out sampled against the wrong rest pose.
    keep = {a.name for a in baked_actions}
    for action in list(bpy.data.actions):
        if action.name not in keep:
            bpy.data.actions.remove(action)

    bpy.ops.object.select_all(action='DESELECT')
    rig.select_set(True)
    bpy.context.view_layer.objects.active = rig
    rig.animation_data.action = baked_actions[0]
    bpy.ops.export_armature.anm(directory=options["out"], as_text=options["text"])

    extension = ".txt" if options["text"] else ".anm"
    written = [f for f in sorted(os.listdir(options["out"])) if f.endswith(extension)]
    print("=== sidekick_anim: {} bones, wrote {} ===".format(len(rig.data.bones), written))


if __name__ == "__main__":
    main()
