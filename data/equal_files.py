import os
import shutil

def count_files_in_subfolders(root_dir):
    subfolder_file_counts = {}
    for subdir, _, files in os.walk(root_dir):
        if subdir == root_dir:
            continue
        subfolder_file_counts[subdir] = len(files)
    return subfolder_file_counts

def copy_files_to_match_max(root_dir, subfolder_file_counts):
    max_files = max(subfolder_file_counts.values())
    for subdir, file_count in subfolder_file_counts.items():
        if file_count == 0:
            print(f"Skipping {subdir} as it has zero files.")
            continue
        if file_count < max_files:
            files_to_copy = max_files - file_count
            files = os.listdir(subdir)
            for i in range(files_to_copy):
                src_file = os.path.join(subdir, files[i % file_count])
                dst_file = os.path.join(subdir, f"{os.path.splitext(files[i % file_count])[0]}_copy_{i}{os.path.splitext(files[i % file_count])[1]}")
                shutil.copy(src_file, dst_file)
            print(f"Copied {files_to_copy} files to {subdir} to match the max file count.")

def main():
    root_dir = './'
    subfolder_file_counts = count_files_in_subfolders(root_dir)
    copy_files_to_match_max(root_dir, subfolder_file_counts)

if __name__ == "__main__":
    main()
