#!/bin/bash

sudo apt update
sudo apt install -y uuid-dev clang llvm gcc-aarch64-linux-gnu lld ca-certificates gnupg make

#cargo install --force cargo-make
#cargo add cargo-tarpaulin

echo "Installing Linux NuGet compatibility shim..."

sudo tee /usr/local/bin/nuget >/dev/null <<'EOF'
#!/usr/bin/env python3

import os
import shutil
import subprocess
import sys
import tempfile

args = sys.argv[1:]

print("[nuget-shim]", " ".join(args), file=sys.stderr)

#
# nuget locals global-packages -list
#
if len(args) >= 3 and \
   args[0] == "locals" and \
   args[1] == "global-packages":

    cache = os.path.expanduser("~/.nuget/packages")
    print(f"global-packages: {cache}")
    sys.exit(0)

#
# nuget install <package>
#
if len(args) >= 2 and args[0] == "install":

    package = args[1]

    version = None
    source = None
    output_dir = None

    i = 2
    while i < len(args):

        if args[i] == "-Version":
            version = args[i + 1]
            i += 2
            continue

        if args[i] == "-Source":
            source = args[i + 1]
            i += 2
            continue

        if args[i] == "-OutputDirectory":
            output_dir = args[i + 1].strip('"')
            i += 2
            continue

        i += 1

    if not package:
        print("Missing package name", file=sys.stderr)
        sys.exit(1)

    if not source:
        print("Missing source", file=sys.stderr)
        sys.exit(1)

    if not version:
        print("Missing version", file=sys.stderr)
        sys.exit(1)

    if not output_dir:
        print("Missing output directory", file=sys.stderr)
        sys.exit(1)

    print(f"[nuget-shim] package={package}", file=sys.stderr)
    print(f"[nuget-shim] version={version}", file=sys.stderr)
    print(f"[nuget-shim] source={source}", file=sys.stderr)
    print(f"[nuget-shim] output={output_dir}", file=sys.stderr)

    work = tempfile.mkdtemp(prefix="mu_nuget_")

    try:

        proj = os.path.join(work, "restore.csproj")

        with open(proj, "w") as f:
            f.write(f"""
<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <TargetFramework>net8.0</TargetFramework>
  </PropertyGroup>

  <ItemGroup>
    <PackageReference Include="{package}"
                      Version="{version}" />
  </ItemGroup>
</Project>
""")

        cfg = os.path.join(work, "nuget.config")

        with open(cfg, "w") as f:
            f.write(f"""<?xml version="1.0" encoding="utf-8"?>
<configuration>
  <packageSources>
    <clear />
    <add key="feed" value="{source}" />
  </packageSources>
</configuration>
""")

        cmd = [
            "dotnet",
            "restore",
            proj,
            "--configfile",
            cfg,
            "-v",
            "minimal"
        ]

        print("[nuget-shim] running:", " ".join(cmd), file=sys.stderr)

        result = subprocess.run(cmd)

        if result.returncode != 0:
            print(
                f"[nuget-shim] restore failed rc={result.returncode}",
                file=sys.stderr
            )
            sys.exit(result.returncode)

        cache_root = os.path.expanduser("~/.nuget/packages")

        package_cache = os.path.join(
            cache_root,
            package.lower(),
            version.lower()
        )

        print(
            f"[nuget-shim] cache path: {package_cache}",
            file=sys.stderr
        )

        if not os.path.isdir(package_cache):
            print(
                f"[nuget-shim] package not found in cache",
                file=sys.stderr
            )
            sys.exit(1)

        target = os.path.join(output_dir, package)

        print(
            f"[nuget-shim] copying -> {target}",
            file=sys.stderr
        )

        shutil.copytree(
            package_cache,
            target,
            dirs_exist_ok=True
        )

        sys.exit(0)

    finally:
        shutil.rmtree(work, ignore_errors=True)

print(
    "[nuget-shim] unsupported command: " + " ".join(args),
    file=sys.stderr
)
sys.exit(1)
EOF

sudo chmod +x /usr/local/bin/nuget

sudo tee /usr/local/bin/mono >/dev/null <<'EOF'
#!/bin/sh

shift
exec nuget "$@"
EOF

sudo chmod +x /usr/local/bin/mono


# Install and Upgrade all needed python packages
pip install --upgrade -r ./pip-requirements.txt

export BOARD=${BOARD:-SurfaceDuo1}
python3 ./Platforms/AndromedaPkg/PlatformBuild.py --setup -t RELEASE
python3 ./Platforms/AndromedaPkg/PlatformBuild.py --update -t RELEASE
