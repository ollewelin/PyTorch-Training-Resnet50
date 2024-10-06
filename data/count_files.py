import os

def count_files_in_subdirectories(directory):
    subdirectory_file_count = {}
    
    for root, dirs, files in os.walk(directory):
        subdirectory_file_count[root] = len(files)
    
    if subdirectory_file_count:
        # Find the subdirectory with the highest number of files
        max_files_subdirectory = max(subdirectory_file_count, key=subdirectory_file_count.get)
        
        print(f"The subdirectory with the highest number of files is: {max_files_subdirectory}")
        print(f"Number of files: {subdirectory_file_count[max_files_subdirectory]}")
    else:
        print("No files found in the specified directory.")

# Replace '/path/to/directory' with the path to the directory you want to check
count_files_in_subdirectories('./')
