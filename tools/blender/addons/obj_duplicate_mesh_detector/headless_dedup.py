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

# Headless duplicate-mesh analysis / dedup script.
#
# Usage:
#   blender <file.blend> --background --python headless_dedup.py -- \
#       [--analyze | --apply] \
#       [--tolerance 0.0001] [--threshold 100.0] \
#       [--allow-scale] [--allow-non-uniform-scale] \
#       [--world-space] [--save]
#
# --analyze : print statistics only (no modification, no save).
# --apply   : detect, convert to linked data, optionally save.

import bpy
import sys
import os
import argparse
import time
from collections import defaultdict
import numpy as np

# Make the addon importable regardless of where Blender's user addons live.
ADDON_DIR = os.path.dirname(os.path.abspath(__file__))
PARENT = os.path.dirname(ADDON_DIR)
if PARENT not in sys.path:
    sys.path.insert(0, PARENT)

from obj_duplicate_mesh_detector import detector  # noqa: E402


def parse_args():
    argv = sys.argv
    if "--" in argv:
        argv = argv[argv.index("--") + 1:]
    else:
        argv = []
    p = argparse.ArgumentParser()
    p.add_argument("--analyze", action="store_true")
    p.add_argument("--apply", action="store_true")
    p.add_argument("--tolerance", type=float, default=0.0001)
    p.add_argument("--threshold", type=float, default=100.0)
    p.add_argument("--allow-scale", action="store_true")
    p.add_argument("--allow-non-uniform-scale", action="store_true")
    p.add_argument("--world-space", action="store_true")
    p.add_argument("--compare-uvs", action="store_true")
    p.add_argument("--save", action="store_true")
    p.add_argument("--save-as", type=str, default=None)
    p.add_argument("--mode", choices=["umeyama", "hash", "centroid", "radial"], default="hash",
                   help="Detection strategy. "
                        "'hash'     : local-space quantized point-cloud hash + face/edge count. "
                        "'centroid' : translation-invariant (centroid-subtracted) hash. "
                        "'radial'   : rotation+translation invariant fingerprint "
                        "(sorted vertex distances from centroid + face/edge counts). "
                        "'umeyama'  : the addon's index-correspondence path.")
    return p.parse_args(argv)


def _fingerprint_local(pc, tolerance):
    return detector.compute_point_cloud_hash(pc, tolerance)


def _fingerprint_centroid(pc, tolerance):
    arr = np.asarray(pc, dtype=np.float64)
    if arr.size == 0:
        return (0,)
    arr = arr - arr.mean(axis=0)
    return detector.compute_point_cloud_hash([tuple(r) for r in arr], tolerance)


def _fingerprint_radial(pc, tolerance):
    """
    Rotation+translation invariant fingerprint: quantized, sorted distances
    of every vertex from the centroid. Two meshes that are rigid copies of
    each other will produce the same fingerprint (subject to quantization).
    """
    arr = np.asarray(pc, dtype=np.float64)
    if arr.size == 0:
        return (0,)
    arr = arr - arr.mean(axis=0)
    dists = np.sqrt((arr * arr).sum(axis=1))
    step = max(tolerance, 1e-12)
    q = np.round(dists / step).astype(np.int64)
    q.sort()
    return (len(q), tuple(q.tolist()))


FINGERPRINTERS = {
    "hash": _fingerprint_local,
    "centroid": _fingerprint_centroid,
    "radial": _fingerprint_radial,
}


def find_duplicates_by_fingerprint(objects, tolerance, mode):
    """
    Group objects whose meshes match under `mode`'s fingerprint, plus
    face & edge counts (cheap topology check). Returns (groups, info).
    """
    fp = FINGERPRINTERS[mode]
    buckets = defaultdict(list)
    for o in objects:
        pc = detector.get_point_cloud(o, use_world_space=False)
        key = (fp(pc, tolerance),
               len(o.data.polygons),
               len(o.data.edges))
        buckets[key].append(o)

    groups = []
    already_linked_within = 0
    for objs in buckets.values():
        if len(objs) < 2:
            continue
        seen_data = set()
        unique_by_data = []
        for o in objs:
            if o.data.name in seen_data:
                already_linked_within += 1
                continue
            seen_data.add(o.data.name)
            unique_by_data.append(o)
        if len(unique_by_data) < 2:
            continue
        groups.append([o.name for o in unique_by_data])

    info = {
        "buckets_with_duplicates": sum(1 for v in buckets.values() if len(v) > 1),
        "groups": len(groups),
        "already_linked_skipped_in_buckets": already_linked_within,
    }
    return groups, info


def collect_mesh_objects():
    return [o for o in bpy.data.scenes[0].objects if o.type == 'MESH' and o.data]


def print_pre_stats(objects, tolerance):
    print("\n========== PRE-DETECTION STATISTICS ==========")
    print(f"Total mesh objects: {len(objects)}")

    unique_meshes = {o.data.name for o in objects}
    print(f"Unique mesh datablocks (already linked): {len(unique_meshes)}")
    print(f"Objects that COULD be deduplicated at best: "
          f"{len(objects) - len(unique_meshes)}")

    # Bucket by mesh-data identity (already-linked groups).
    data_groups = defaultdict(list)
    for o in objects:
        data_groups[o.data.name].append(o.name)
    already_linked = {k: v for k, v in data_groups.items() if len(v) > 1}
    print(f"Already-linked groups: {len(already_linked)}")

    # Bucket by vertex count (loose dedup candidates).
    vc = defaultdict(list)
    for o in objects:
        vc[len(o.data.vertices)].append(o.name)
    multi_vc = {k: v for k, v in vc.items() if len(v) > 1}
    print(f"Vertex-count buckets with >1 object: {len(multi_vc)}")
    candidate_pairs = sum(len(v) * (len(v) - 1) // 2 for v in multi_vc.values())
    print(f"Candidate pair comparisons (vertex-count bucketed): {candidate_pairs}")

    # Hash-collision buckets (true local-space hash duplicates).
    hash_groups = defaultdict(list)
    for o in objects:
        pc = detector.get_point_cloud(o, use_world_space=False)
        h = detector.compute_point_cloud_hash(pc, tolerance)
        hash_groups[h].append(o.name)
    hash_dups = {k: v for k, v in hash_groups.items() if len(v) > 1}
    print(f"Hash-identical groups (local space, tol={tolerance}): {len(hash_dups)}")
    objs_in_hash_dups = sum(len(v) for v in hash_dups.values())
    print(f"Objects in hash-identical groups: {objs_in_hash_dups}")

    # Top vertex-count buckets to give a feel for the data.
    top = sorted(multi_vc.items(), key=lambda kv: -len(kv[1]))[:10]
    if top:
        print("\nTop vertex-count buckets:")
        for vcount, names in top:
            print(f"  verts={vcount:>6}  count={len(names):>4}  "
                  f"e.g. {names[0]}")
    print("==============================================\n")


def run_detection(objects, args):
    t0 = time.time()
    if args.mode == "umeyama":
        groups, scale_logs = detector.find_duplicate_meshes(
            objects,
            tolerance=args.tolerance,
            use_world_space=args.world_space,
            compare_uvs=args.compare_uvs,
            match_threshold=args.threshold,
            allow_scale=args.allow_scale,
            allow_non_uniform_scale=args.allow_non_uniform_scale,
        )
        dt = time.time() - t0
        print(f"\n=== Umeyama detection finished in {dt:.2f}s ===")
    else:
        groups, info = find_duplicates_by_fingerprint(
            objects, args.tolerance, args.mode)
        scale_logs = []
        dt = time.time() - t0
        print(f"\n=== '{args.mode}' fingerprint detection finished in {dt:.2f}s ===")
        print(f"Buckets with >1 object: {info['buckets_with_duplicates']}")
        print(f"Already-linked entries skipped within buckets: "
              f"{info['already_linked_skipped_in_buckets']}")

    print(f"Duplicate groups: {len(groups)}")
    total_in_groups = sum(len(g) for g in groups)
    print(f"Objects in groups: {total_in_groups}")
    redundant = sum(len(g) - 1 for g in groups)
    print(f"Redundant meshes that could be linked: {redundant}")
    sizes = sorted((len(g) for g in groups), reverse=True)
    if sizes:
        print(f"Group sizes (top 15): {sizes[:15]}")
    for i, g in enumerate(groups, 1):
        print(f"  Group {i} ({len(g)}): {', '.join(g[:6])}"
              f"{' ...' if len(g) > 6 else ''}")
    return groups, scale_logs


def main():
    args = parse_args()
    if not args.analyze and not args.apply:
        args.analyze = True

    print(f"Blender:    {bpy.app.version_string}")
    print(f"Blend file: {bpy.data.filepath}")
    print(f"Args:       tol={args.tolerance} thr={args.threshold} "
          f"world_space={args.world_space} allow_scale={args.allow_scale} "
          f"allow_non_uniform_scale={args.allow_non_uniform_scale} "
          f"compare_uvs={args.compare_uvs}")

    objects = collect_mesh_objects()
    print_pre_stats(objects, args.tolerance)

    groups, _ = run_detection(objects, args)

    if args.apply and groups:
        scene = bpy.data.scenes[0]
        before_meshes = len(bpy.data.meshes)
        converted = detector.convert_to_linked(groups, scene)
        after_meshes = len(bpy.data.meshes)
        print(f"\n=== Converted {converted} objects to linked data ===")
        print(f"Mesh datablocks: {before_meshes} -> {after_meshes} "
              f"(removed {before_meshes - after_meshes})")

        # Purge fully-orphaned data just in case.
        bpy.ops.outliner.orphans_purge(do_local_ids=True,
                                       do_linked_ids=True,
                                       do_recursive=True)
        print(f"After orphan purge: {len(bpy.data.meshes)} mesh datablocks")

        if args.save_as:
            bpy.ops.wm.save_as_mainfile(filepath=args.save_as)
            print(f"Saved to {args.save_as}")
        elif args.save:
            bpy.ops.wm.save_mainfile()
            print(f"Saved {bpy.data.filepath}")
    elif args.apply:
        print("Nothing to convert.")


if __name__ == "__main__":
    main()
