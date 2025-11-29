#!/usr/bin/env python3
"""
WLED Firmware Organizer
Copies and sorts compiled firmware files from 'build_output/release' to 'build_output/custom' folder by categories and versions.
Run this after: npm run build && pio run
"""

import os
import shutil
import glob
from pathlib import Path
from collections import defaultdict
import re

def extract_version_from_filename(filename):
  """Extract WLED version from firmware filename"""
  # Expected patterns: WLED_0.15.1_env.bin, WLED_0.16.0-alpha_env.bin
  version_match = re.search(r'WLED_([0-9]+\.[0-9]+\.[0-9]+(?:-[^_]+)?)_', filename)
  if version_match:
    return version_match.group(1)
  
  # Fallback patterns for different naming conventions
  legacy_match = re.search(r'([0-9]+\.[0-9]+\.[0-9]+(?:-[^_]+)?)', filename)
  if legacy_match:
    return legacy_match.group(1)
  
  return None

def get_wled_version_from_header():
  """Extract WLED version from wled.h"""
  try:
    wled_h_path = Path('wled00/wled.h')
    if wled_h_path.exists():
      with open(wled_h_path, 'r') as f:
        content = f.read()
        version_match = re.search(r'#define\s+WLED_VERSION\s+"([^"]+)"', content)
        if version_match:
          return version_match.group(1)
  except Exception:
    pass
  return "0.16.0-alpha"

def create_directory_structure(base_dir, version):
  """Create the organized directory structure for a specific version"""
  version_dir = base_dir / version
  categories = [
    '4ch_board',
    'Debug_builds', 
    'esp32_wled_dev_board',
    'mini_shield',
    'S2_S3_C3',
    'universal_shield'
  ]
  
  for category in categories:
    Path(version_dir / category).mkdir(parents=True, exist_ok=True)
  
  return version_dir

def categorize_firmware(env_name):
  """Determine category based on environment name patterns"""
  env_lower = env_name.lower()
  
  # 4ch_board builds (fan + relay combinations)
  if any(pattern in env_lower for pattern in ['4ch_fan', 'fan_display_rotary_relays', 'devb_4ch']):
    return '4ch_board'
  
  # Debug builds  
  if any(pattern in env_lower for pattern in ['debug']):
    return 'Debug_builds'
    
  # ESP32 development board builds (Dallas sensor, SHT sensor variants)
  if any(pattern in env_lower for pattern in ['esp32_devb_dallas', 'esp32_devb_sht', 'devb_dallas', 'devb_sht']):
    return 'esp32_wled_dev_board'
  
  # Mini shield builds (all mini shield variants)
  if any(pattern in env_lower for pattern in ['mini_shield']):
    return 'mini_shield'
    
  # S2/S3/C3 variants - expanded patterns to catch all S2/S3/C3 builds
  if any(pattern in env_lower for pattern in [
    'esp32s2', 's2_', 'saola',           # ESP32-S2 variants
    'esp32s3', 's3_', 's3dev',           # ESP32-S3 variants  
    'esp32c3', 'c3_', 'stamp5_c3',       # ESP32-C3 variants
    'lolin_s2', 'lolin_s3', 'lolin_c3'   # Lolin board variants
  ]):
    return 'S2_S3_C3'
  
  # Universal shield builds - ONLY files with "universal" in name
  if 'universal' in env_lower:
    return 'universal_shield'
  
  # All other files go to root version folder (no category)
  return None

def organize_firmware():
  """Main function to organize firmware files"""
  # WLED build system places compiled firmware in build_output/release
  build_dir = Path('build_output/release')
  dest_base_dir = Path('build_output/custom')
  
  # Check if build directory exists
  if not build_dir.exists():
    print(f"ERROR: Build directory '{build_dir}' not found!")
    print("INFO: Make sure to run 'npm run build && pio run' first")
    return False
  
  # Find all firmware files (.bin files)
  firmware_files = list(build_dir.glob('*.bin'))
  
  if not firmware_files:
    print(f"WARNING: No .bin files found in '{build_dir}'")
    print("INFO: Run 'pio run' to build firmware first")
    return False
  
  # Group firmware files by version
  version_groups = defaultdict(list)
  unknown_version_files = []
  
  for firmware_path in firmware_files:
    filename = firmware_path.name
    version = extract_version_from_filename(filename)
    
    if version:
      version_groups[version].append(firmware_path)
    else:
      unknown_version_files.append(firmware_path)
  
  # If no versions detected from filenames, use header version as fallback
  if not version_groups and unknown_version_files:
    header_version = get_wled_version_from_header()
    print(f"No version detected from filenames, using header version: {header_version}")
    version_groups[header_version] = unknown_version_files
    unknown_version_files = []
  
  if not version_groups:
    print("ERROR: No firmware files with recognizable versions found!")
    return False
  
  print(f"Found {len(firmware_files)} firmware files across {len(version_groups)} versions:")
  for version, files in version_groups.items():
    print(f"  Version {version}: {len(files)} files")
  
  # Process each version separately
  total_organized = 0
  
  for version, version_files in version_groups.items():
    print(f"\nProcessing version {version}...")
    
    # Create directory structure for this version
    version_dir = create_directory_structure(dest_base_dir, version)
    
    # Organize files by category for this version
    categorized_files = defaultdict(list)
    root_files = []
    
    for firmware_path in version_files:
      filename = firmware_path.name
      
      # Extract environment name from filename
      env_name = filename
      if filename.startswith(f"WLED_{version}_"):
        env_name = filename[len(f"WLED_{version}_"):].replace('.bin', '')
      elif filename.endswith('.bin'):
        env_name = filename.replace('.bin', '')
      
      category = categorize_firmware(env_name)
      
      if category is None:
        # Place uncategorized files in root version folder
        dest_file = version_dir / filename
        root_files.append((filename, firmware_path))
        try:
          shutil.copy2(firmware_path, dest_file)
          print(f"  SUCCESS: {filename} -> {version}/{filename}")
          total_organized += 1
        except Exception as e:
          print(f"  ERROR: Failed to copy {filename}: {e}")
      else:
        # Place categorized files in category subfolder
        categorized_files[category].append((filename, firmware_path))
        dest_file = version_dir / category / filename
        try:
          shutil.copy2(firmware_path, dest_file)
          print(f"  SUCCESS: {filename} -> {version}/{category}/{filename}")
          total_organized += 1
        except Exception as e:
          print(f"  ERROR: Failed to copy {filename}: {e}")
    
    # Display summary for this version
    print(f"\nVersion {version} Summary:")
    print("-" * 40)
    
    if root_files:
      print(f"  Root files: {len(root_files)}")
      for filename, _ in root_files[:2]:
        print(f"    - {filename}")
      if len(root_files) > 2:
        print(f"    - ... and {len(root_files)-2} more")
    
    for category in sorted(categorized_files.keys()):
      count = len(categorized_files[category])
      print(f"  {category}: {count} files")
      for filename, _ in categorized_files[category][:2]:
        print(f"    - {filename}")
      if count > 2:
        print(f"    - ... and {count-2} more")
  
  # Handle unknown version files
  if unknown_version_files:
    print(f"\nWARNING: {len(unknown_version_files)} files with unrecognized version patterns:")
    for file_path in unknown_version_files:
      print(f"  - {file_path.name}")
  
  print("\n" + "=" * 60)
  print(f"Total files organized: {total_organized}")
  print(f"Versions processed: {', '.join(sorted(version_groups.keys()))}")
  print(f"Files organized into: {dest_base_dir.absolute()}")
  
  return True

def clean_custom_folder():
  """Clean the custom folder before organizing"""
  dest_dir = Path('build_output/custom')
  if dest_dir.exists():
    print("Cleaning existing custom folder...")
    shutil.rmtree(dest_dir)

def main():
  """Main entry point"""
  print("WLED Firmware Organizer")
  print("=" * 60)
  
  # Optional: clean before organizing
  import sys
  if '--clean' in sys.argv:
    clean_custom_folder()
  
  success = organize_firmware()
  
  if success:
    print("\nFirmware organization completed successfully!")
    print("\nNext steps:")
    print("   - Upload firmware files from build_output/custom/ folders to your devices")
    print("   - Use PlatformIO to flash specific builds: pio run -e <env> --target upload")
  else:
    print("\nFirmware organization failed!")
    return 1
  
  return 0

if __name__ == "__main__":
  exit(main())