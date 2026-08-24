# AeonTool

AeonTool is a command-line utility suite for the AeonEngine game engine. It provides various tools for managing and converting game assets and resources.

## Overview

AeonTool is a multi-purpose command-line tool that operates using subcommands. Each subcommand provides specific functionality for working with different types of game engine assets.

## Usage

```text
aeontool <tool> [options]
```

Where `<tool>` is one of: `convert`, `pack`, `base64`, `pipeline`, `index`,
`sidekick`, or `sidekickdb`.

## Available Tools

### 1. Convert

Converts between binary and text formats of AeonEngine resource files.

**Supported File Types:**

- **Pipeline files** (`.pln` / `.txt`) - Graphics pipeline definitions containing shader code
- **Material files** (`.mtl` / `.txt`) - Material property definitions
- **Mesh files** (`.msh` / `.txt`) - 3D mesh geometry data
- **Skeleton files** (`.skl` / `.txt`) - Skeletal animation data
- **Scene files** (`.scn` / `.txt`) - Scene graph and component data
- **Collision files** (`.cln` / `.txt`) - Collision geometry
- **Model files** (`.mdl` / `.txt`) - Mesh/material assemblies and animation references
- **Animation files** (`.anm` / `.txt`) - Skeletal animation clips

**Usage:**

```text
aeontool convert [options]
```

**Options:**

- `-i <input>` or `--in <input>` - Specify input file path
- `-o <output>` or `--out <output>` - Specify output file path
- If no flags are provided, the first argument is treated as the input file

**Functionality:**

- Automatically detects file format (binary or text) based on magic number
- Binary to text: Converts binary resource files to human-readable text format
- Text to binary: Converts text resource files to optimized binary format
- Special handling for mesh files:
  - Vertex buffers are formatted in a human-readable format
  - Index buffers are formatted for easy editing
  - Parses text representations of buffers back to binary data
- Pipeline files: Shader code is formatted for readability

**Examples:**

```bash
# Convert binary pipeline to text
aeontool convert -i shader.pln -o shader.txt

# Convert text mesh to binary
aeontool convert mesh.txt -o mesh.msh
```

---

### 2. Pack

Manages AeonEngine package files (`.pkg`) for asset compression and archival.

**Usage:**

```text
aeontool pack [options]
```

**Options:**

- `-i <path>` or `--in <path>` - Specify input path (file or directory)
- `-o <file>` or `--out <file>` - Specify output package file
- `-c` or `--compress` - Compress files into a package
- `-e` or `--extract` - Extract files from a package
- `-d` or `--directory` - List contents of a package
- `--store` - Store files without zlib compression

**Actions:**

#### Compress

Compresses files and directories into a `.pkg` package file using zlib compression.

```bash
aeontool pack --compress -i /path/to/assets -o game.pkg
```

#### Extract

Extracts every package entry and recreates its relative directory structure. If
`--out` is omitted, the package filename without its extension is used.

```bash
aeontool pack --extract -i game.pkg -o extracted-game
```

#### Directory

Lists all entries in a package file with their identifiers.

```bash
aeontool pack --directory -i game.pkg
```

**Features:**

- Uses zlib compression for efficient storage
- Maintains string table for resource identification
- Supports CRC-based file indexing
- Directory traversal for batch packaging

---

### 3. Base64

Encodes and decodes files using Base64 encoding.

**Usage:**

```text
aeontool base64 <encode|decode> [options]
```

**Subcommands:**

- `encode` - Encode a file to Base64 format
- `decode` - Decode a Base64-encoded file

**Options:**

- `-i <input>` or `--in <input>` - Specify input file path
- `-o <output>` or `--out <output>` - Specify output file path
- If no flags are provided, the first argument after encode/decode is treated as the input file

**Functionality:**

- Encode any file to Base64 text format
- Decode Base64 text back to original binary format
- Automatic output file naming for encoding (adds `.b64` extension)
- Decoding requires explicit output file specification

**Examples:**

```bash
# Encode a binary file
aeontool base64 encode -i image.png -o image.b64

# Decode back to original format
aeontool base64 decode -i image.b64 -o image.png
```

---

### 4. Pipeline

Packs shader sources into pipeline resources and extracts sources from existing
pipeline files. A pipeline may contain graphics stages and multiple ordered
compute stages.

**Usage:**

```text
aeontool pipeline [options]
```

**Options:**

- `-i <input>` or `--in <input>` - Specify input file path (shader base name without extension)
- `-o <output>` or `--out <output>` - Specify output file path (`.pln` or `.txt`)
- `--vert`, `--frag`, `--comp`, `--tesc`, `--tese`, `--geom` - Set an explicit source for one stage; repeat `--comp` for ordered compute stages
- `--variant <stage> <renderer-set> <file>` - Override a stage for a comma-separated set such as `Vulkan,Metal`
- `--disable <stage> <renderer-set>` - Disable a stage for the selected renderers
- `--topology <class>` - Set `TRIANGLE`, `LINE`, `POINT`, or `PATCH`
- `-h` or `--help` - Show the complete built-in command help
- If no flags are provided, the first argument is treated as the input file

**Supported Shader Types:**

- `.vert` - Vertex shaders
- `.frag` - Fragment shaders
- `.comp` - Compute shaders
- `.tesc` - Tessellation control shaders
- `.tese` - Tessellation evaluation shaders
- `.geom` - Geometry shaders

**Functionality:**

- Automatically searches for shader files with supported extensions
- Bundles multiple shader stages into a single pipeline resource
- Stores renderer-scoped variants while retaining a default source for other renderers
- Extracts embedded sources when the input ends in `.pln` or `.txt`
- Outputs to binary (`.pln`) or text (`.txt`) format
- Input file should be specified without extension (tool will find all matching shader files)

**How it Works:**

1. Provide a base name without extension (e.g., `my_shader`)
2. Tool searches for all matching shader files:
   - `my_shader.vert`
   - `my_shader.frag`
   - `my_shader.comp`
   - etc.
3. Found shaders are compiled into a single pipeline resource
4. Output format determined by output file extension

**Examples:**

```bash
# Create binary pipeline from shader files
# Looks for: phong.vert, phong.frag, etc.
aeontool pipeline -i phong -o phong.pln

# Create text pipeline for easy editing
aeontool pipeline -i pbr -o pbr.txt

# Share a multiview vertex shader between Vulkan and Metal and disable geometry there
aeontool pipeline --vert point_shadow_depth.vert \
  --geom point_shadow_depth.geom \
  --frag point_shadow_depth.frag \
  --variant vert Vulkan,Metal point_shadow_depth_mv.vert \
  --disable geom Vulkan,Metal \
  -o point_shadow_depth.txt

# Extract a packed pipeline's shader sources
aeontool pipeline -i point_shadow_depth.pln -o extracted/point_shadow_depth
```

---

### 5. Index

Builds a deterministic CRC32-to-path index for a cooked game directory. Paths
are relative to the root, normalized to forward slashes, and sorted by CRC in
the binary form.

```bash
aeontool index [options] [root]
```

**Options:**

- `-i <dir>`, `--in <dir>`, or `--root <dir>` - Root directory (default: `game`)
- `-o <file>` or `--out <file>` - Output file
- `-b` or `--binary` - Write binary `AEONIDX`; the default is tab-separated text

Without `--out`, text output goes to `<root>/index.txt` and binary output to
`<root>/index.idx`.

```bash
aeontool index --root game --binary
```

---

### 6. SIDEKICK Character Recipe

Converts a Synty SIDEKICK `.sk` character recipe plus `Synty_Sidekick.db` into
an AeonEngine model, material, and baked 32x32 palette textures. Geometry is
handled separately by the Blender SIDEKICK scripts and exporters because
AeonTool does not read FBX.

> **Licensed content:** SIDEKICK packages, extracted files, and every cooked or
> baked derivative are proprietary assets and must not be committed to this
> public repository. Keep inputs and outputs under the ignored `synty/` tree or
> outside the worktree.

```bash
aeontool sidekick -i character.sk -d Synty_Sidekick.db [options]
```

Important options include `--out <dir>`, `--prefix <resource-path>`,
`--textures <dir>`, `--name <name>`, `--pipeline <path>`, `--skeleton <path>`,
`--mesh-extension <ext>`, repeatable `--animation <name>=<file>`, and `--binary`.
Run `aeontool sidekick --help` for defaults and the complete reference.

---

### 7. SIDEKICK Character Database

Transcodes `Synty_Sidekick.db` into the engine-owned character-library schema.
When `--meshes` is supplied, parts without a cooked mesh are removed along with
outfits that require them, preventing the runtime from offering incomplete
characters. The licensed-content restriction above applies to this output too.

```bash
aeontool sidekickdb -i Synty_Sidekick.db -o character-library.db [options]
```

Options configure the cooked mesh directory/resource prefix, mesh extension,
skeleton, pipeline, material, and repeatable shared animations. Run
`aeontool sidekickdb --help` for the complete option list and defaults.

---

## File Format Specifications

### Magic Numbers

AeonEngine files use magic numbers for format identification. Each is seven
characters followed by `\0` in binary files and by a newline in text files:

- `AEONPLN` - Pipeline files
- `AEONMTL` - Material files
- `AEONMSH` - Mesh files
- `AEONSKL` - Skeleton files
- `AEONSCN` - Scene files
- `AEONCLN` - Collision files
- `AEONMDL` - Model files
- `AEONANM` - Animation files
- `AEONPKG` - Package archives
- `AEONIDX` - Binary CRC32-to-path indexes

### Binary vs Text Formats

- **Binary protobuf resources**: Optimized for runtime loading and smaller file size
- **Text protobuf resources**: Human-readable, suitable for version control and manual editing
- The binary and text forms of protobuf resources contain equivalent data; packages and indexes use their own formats

## Dependencies

- **Google Protocol Buffers**: Used for serialization/deserialization
- **zlib**: Used for compression in package files
- **C++20**: Required for compilation

## Building

AeonTool is built as part of the AeonEngine project. It's located in the `tools/aeontool` directory and is compiled using CMake.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target aeontool
```

The compiled binary will be located in `build/bin/aeontool` (or `aeontool.exe` on Windows).

## Error Handling

All tools provide descriptive error messages for common issues:

- Missing input files
- Invalid file formats
- Incorrect command-line arguments
- File I/O errors

Errors are printed to standard output and the tool exits with a non-zero status code.

## License

Copyright (C) 2016-2026 Rodrigo Jose Hernandez Cordoba

Licensed under the Apache License, Version 2.0. See [LICENSE.md](../../LICENSE.md) for details.
