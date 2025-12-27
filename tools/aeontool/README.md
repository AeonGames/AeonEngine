# AeonTool

AeonTool is a command-line utility suite for the AeonEngine game engine. It provides various tools for managing and converting game assets and resources.

## Overview

AeonTool is a multi-purpose command-line tool that operates using subcommands. Each subcommand provides specific functionality for working with different types of game engine assets.

## Usage

```
aeontool <tool> [options]
```

Where `<tool>` is one of: `convert`, `pack`, `base64`, or `pipeline`.

## Available Tools

### 1. Convert

Converts between binary and text formats of AeonEngine resource files.

**Supported File Types:**
- **Pipeline files** (`.pln` / `.txt`) - Graphics pipeline definitions containing shader code
- **Material files** (`.mtl` / `.txt`) - Material property definitions
- **Mesh files** (`.msh` / `.txt`) - 3D mesh geometry data
- **Skeleton files** (`.skl` / `.txt`) - Skeletal animation data

**Usage:**
```
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
```
aeontool pack [options]
```

**Options:**
- `-i <path>` or `--in <path>` - Specify input path (file or directory)
- `-o <file>` or `--out <file>` - Specify output package file
- `-c` or `--compress` - Compress files into a package
- `-e` or `--extract` - Extract files from a package
- `-d` or `--directory` - List contents of a package

**Actions:**

#### Compress
Compresses files and directories into a `.pkg` package file using zlib compression.
```bash
aeontool pack --compress -i /path/to/assets -o game.pkg
```

#### Extract
Extracts contents from a package file (currently not fully implemented).
```bash
aeontool pack --extract -i game.pkg
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
```
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

Creates and manages graphics pipeline resource files from shader source code.

**Usage:**
```
aeontool pipeline [options]
```

**Options:**
- `-i <input>` or `--in <input>` - Specify input file path (shader base name without extension)
- `-o <output>` or `--out <output>` - Specify output file path (`.pln` or `.txt`)
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
```

---

## File Format Specifications

### Magic Numbers
AeonEngine files use magic numbers for format identification:
- `AEONPLN` - Pipeline files (binary with `\0`, text without)
- `AEONMTL` - Material files (binary with `\0`, text without)
- `AEONMSH` - Mesh files (binary with `\0`, text without)
- `AEONSKLB` - Skeleton binary files
- `AEONSKLT` - Skeleton text files

### Binary vs Text Formats
- **Binary format**: Optimized for runtime loading, smaller file size
- **Text format**: Human-readable, suitable for version control and manual editing
- Both formats contain identical data, just different representations

## Dependencies

- **Google Protocol Buffers**: Used for serialization/deserialization
- **zlib**: Used for compression in package files
- **C++20**: Required for compilation

## Building

AeonTool is built as part of the AeonEngine project. It's located in the `tools/aeontool` directory and is compiled using CMake.

```bash
cd build
cmake ..
make aeontool
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

Copyright (C) 2016-2025 Rodrigo Jose Hernandez Cordoba

Licensed under the Apache License, Version 2.0. See LICENSE for details.
